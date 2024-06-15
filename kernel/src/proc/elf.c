#include <kernel/proc/elf.h>
#include <kernel/proc/task.h>
#include <kernel/kmm.h>
#include <kernel/string.h>

static int verify_elf_header(elf_file_t *elf_file)
{
    if (elf_file->header.e_type != ET_EXEC)
    {
        return -1;
    }

    if (elf_file->header.e_machine != EM_X86_64)
    {
        return -2;
    }

    if (elf_file->header.e_version != EV_CURRENT)
    {
        return -3;
    }

    if (elf_file->header.e_ident[0] != 0x7f || elf_file->header.e_ident[1] != 'E' || elf_file->header.e_ident[2] != 'L' || elf_file->header.e_ident[3] != 'F')
    {
        return -4;
    }

    if (elf_file->header.e_ident[4] != ELFCLASS64)
    {
        return -5;
    }

    if (elf_file->header.e_ident[5] != ELFDATA2LSB)
    {
        return -6;
    }

    if (elf_file->header.e_ident[6] != EV_CURRENT)
    {
        return -7;
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

    elf_file->entry_addr = elf_file->header.e_entry;

    return elf_file;
}

int elf_free(elf_file_t *elf_file)
{
    if (!elf_file || !elf_file->node || !elf_file->program_header_table)
    {
        return -1;
    }

    if (vfs_close(elf_file->node) < 0)
    {
        return -1;
    }

    kfree(elf_file->program_header_table);
    kfree(elf_file);

    return 0;
}

static int load_segment(elf_file_t *elf_file, Elf64_Phdr *ph, process_t *proc, uint64_t *data_pages_index)
{
    if (!ph)
    {
        return -1;
    }

    if (ph->p_type != PT_LOAD)
    {
        return 0;
    }

    if (vfs_seek(elf_file->node, ph->p_offset, SEEK_TYPE_SET) < 0)
    {
        return -1;
    }

    for (size_t i = 0; i < (ph->p_memsz + (PAGE_SIZE - 1)) / PAGE_SIZE; i++)
    {
        proc->data_pages[*data_pages_index] = pmm_alloc();
        if (!proc->data_pages[*data_pages_index])
        {
            return -1;
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

        if (vfs_read(elf_file->node, remainder, proc->data_pages[*data_pages_index]) < 0)
        {
            return -1;
        }

        if (pml4_map(proc->pml4, (void *)(ph->p_vaddr + i * PAGE_SIZE), proc->data_pages[*data_pages_index], PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0) // TODO set permissions with flags
        {
            return -1;
        }

        *data_pages_index += 1;
    }

    return 0;
}

int elf_setup_exec(elf_file_t *elf_file, process_t *proc)
{
    if (!elf_file || !elf_file->node || !elf_file->program_header_table)
    {
        return -1;
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
        return -1;
    }

    uint64_t data_pages_index = 0;
    for (Elf64_Half i = 0; i < elf_file->header.e_phnum; i++)
    {
        if (load_segment(elf_file, &elf_file->program_header_table[i], proc, &data_pages_index) < 0)
        {
            return -1;
        }
    }

    return 0;
}
