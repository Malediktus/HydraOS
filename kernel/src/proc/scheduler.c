#include <kernel/proc/scheduler.h>
#include <kernel/proc/task.h>
#include <kernel/isr.h>
#include <kernel/port.h>

static void timer_irq(interrupt_frame_t *frame)
{
    process_t *proc = get_current_process();
    if (!proc)
    {
        return;
    }

    // save state
    proc->task->state.r15 = frame->r15;
    proc->task->state.r14 = frame->r14;
    proc->task->state.r13 = frame->r13;
    proc->task->state.r12 = frame->r12;
    proc->task->state.r11 = frame->r11;
    proc->task->state.r10 = frame->r10;
    proc->task->state.r9 = frame->r9;
    proc->task->state.r8 = frame->r8;
    proc->task->state.rsi = frame->rsi;
    proc->task->state.rdi = frame->rdi;
    proc->task->state.rbp = frame->rbp;
    proc->task->state.rdx = frame->rdx;
    proc->task->state.rcx = frame->rcx;
    proc->task->state.rbx = frame->rbx;
    proc->task->state.rax = frame->rax;
    proc->task->state.rip = frame->rip;
    proc->task->state.rsp = frame->rsp;

    execute_next_process();
    while (1);
    // TODO: panic
}

int scheduler_init(void)
{
    uint32_t freq = 100;
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);

    return register_interrupt_handler(0x20, timer_irq);
}
