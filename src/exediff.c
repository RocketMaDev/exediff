#include "asm.h"
#include "disasm.h"
#include "rx.h"
#include <stdint.h>
#include <stdio.h>

int
main ()
{
  // map_pages pages = get_rx ("/usr/bin/ls");
  // for (uint32_t i = 0; i < pages.page_num; i++)
  //   printf ("%p 0x%x", (void *)pages.pages[i].page_adr,
  //           pages.pages[i].page_len);
  //
  // free_map_pages (&pages);

  char *code = "mov rax, 0x1234; push rbx; pop rcx; syscall";
  asm_code *encode = assemble (code);
  free_asm (encode);
}
