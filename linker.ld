/* definimos el punto de entrada que esta en head.asm */
ENTRY(_start)

SECTIONS
{
    /* grub carga el kernel a partir de 1 megabyte en la ram física */
    . = 1M;

    /* bloque de código ejecutable principal */
    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    /* datos de solo lectura (cadenas de texto fijas, arrays estáticos) */
    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    /* variables globales inicializadas con algún valor */
    .data : ALIGN(4K)
    {
        *(.data)
    }

    /* variables sin inicializar y el espacio reservado para el stack */
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}
