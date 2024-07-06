#include <kernel/proc/elf.h>
#include <kernel/proc/task.h>
#include <kernel/kmm.h>
#include <kernel/string.h>

static int verify_elf_header(elf_file_t *elf_file)
{
    if (elf_file->header.e_type != ET_EXEC)
    {
        return -ERECOV;
    }

    if (elf_file->header.e_machine != EM_X86_64)
    {
        return -ERECOV;
    }

    if (elf_file->header.e_version != EV_CURRENT)
    {
        return -ERECOV;
    }

    if (elf_file->header.e_ident[0] != 0x7f || elf_file->header.e_ident[1] != 'E' || elf_file->header.e_ident[2] != 'L' || elf_file->header.e_ident[3] != 'F')
    {
        return -ERECOV;
    }

    if (elf_file->header.e_ident[4] != ELFCLASS64)
    {
        return -ERECOV;
    }

    if (elf_file->header.e_ident[5] != ELFDATA2LSB)
    {
        return -ERECOV;
    }

    if (elf_file->header.e_ident[6] != EV_CURRENT)
    {
        return -ERECOV;
    }

    return 0;
}

elf_file_t *elf_load(const char *path)
{
    elf_file_t *elf_file = kmalloc(sizeof(elf_file_t));
    if (!elf_file)
    {
        return NULL;
    }

    elf_file->node = vfs_open(path, OPEN_ACTION_READ);
    if (!elf_file->node)
    {
        kfree(elf_file);
        return NULL;
    }

    if (vfs_read(elf_file->node, sizeof(Elf64_Ehdr), (uint8_t *)&elf_file->header) < 0)
    {
        vfs_close(elf_file->node);
        kfree(elf_file);
        return NULL;
    }

    if (verify_elf_header(elf_file) < 0)
    {
        return NULL;
    }

    if (vfs_seek(elf_file->node, elf_file->header.e_phoff, SEEK_TYPE_SET) < 0)
    {
        vfs_close(elf_file->node);
        kfree(elf_file);
        return NULL;
    }

    elf_file->program_header_table = kmalloc(elf_file->header.e_phnum * sizeof(Elf64_Phdr));
    if (!elf_file->program_header_table)
    {
        vfs_close(elf_file->node);
        kfree(elf_file);
        return NULL;
    }

    if (vfs_read(elf_file->node, elf_file->header.e_phnum * sizeof(Elf64_Phdr), (uint8_t *)elf_file->program_header_table) < 0)
    {
        elf_free(elf_file);
        return NULL;
    }

    return elf_file;
}

int elf_free(elf_file_t *elf_file)
{
    if (!elf_file || !elf_file->node || !elf_file->program_header_table)
    {
        return -EINVARG;
    }

    int status = vfs_close(elf_file->node);
    if (status < 0)
    {
        return status;
    }

    kfree(elf_file->program_header_table);
    kfree(elf_file);

    return 0;
}

static int load_segment(elf_file_t *elf_file, Elf64_Phdr *ph, process_t *proc, uint64_t *data_pages_index)
{
    if (!ph)
    {
        return -EINVARG;
    }

    if (ph->p_type != PT_LOAD)
    {
        return 0;
    }

    int status = vfs_seek(elf_file->node, ph->p_offset, SEEK_TYPE_SET);
    if (status < 0)
    {
        return status;
    }

    for (size_t i = 0; i < (ph->p_memsz + (PAGE_SIZE - 1)) / PAGE_SIZE; i++)
    {
        proc->data_pages[*data_pages_index] = pmm_alloc();
        if (!proc->data_pages[*data_pages_index])
        {
            return -ENOMEM;
        }

        if (ph->p_memsz > ph->p_filesz)
        {
            // uninitialized data
            memset(proc->data_pages[*data_pages_index], 0, PAGE_SIZE);
            *data_pages_index += 1;
            continue;
        }

        size_t remainder = ph->p_memsz - i * PAGE_SIZE;
        if (remainder > PAGE_SIZE)
        {
            remainder = PAGE_SIZE;
        }

        status = vfs_read(elf_file->node, remainder, proc->data_pages[*data_pages_index]);
        if (status < 0)
        {
            return status;
        }

        status = pml4_map(proc->pml4, (void *)(ph->p_vaddr + i * PAGE_SIZE), proc->data_pages[*data_pages_index], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        if (status < 0) // TODO set permissions with flags
        {
            return status;
        }

        *data_pages_index += 1;
    }

    return 0;
}

int elf_setup_exec(elf_file_t *elf_file, process_t *proc)
{
    if (!elf_file || !elf_file->node || !elf_file->program_header_table)
    {
        return -EINVARG;
    }

    uint64_t num_pages = 0;
    for (Elf64_Half i = 0; i < elf_file->header.e_phnum; i++)
    {
        num_pages += (elf_file->program_header_table[i].p_memsz + (PAGE_SIZE - 1)) / PAGE_SIZE;
    }
    
    proc->num_data_pages = num_pages;
    proc->data_pages = kmalloc(num_pages);
    if (!proc->data_pages)
    {
        return -ENOMEM;
    }

    uint64_t data_pages_index = 0;
    for (Elf64_Half i = 0; i < elf_file->header.e_phnum; i++)
    {
        int status = load_segment(elf_file, &elf_file->program_header_table[i], proc, &data_pages_index);
        if (status < 0)
        {
            return status;
        }
    }

    proc->task->state.rip = elf_file->header.e_entry;

    return 0;
}
