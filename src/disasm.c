#include "disasm.h"
#include "log.h"
#include <capstone/capstone.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char *
disassemble (char *to_disasm, uint32_t code_len)
{
  csh handle;
  cs_insn *insn;
  uint64_t count;

  if (cs_open (CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
    PEXIT (CS_OPEN);
  count
      = cs_disasm (handle, (const uint8_t *)to_disasm, code_len, 0, 0, &insn);
  if (count <= 0)
    PEXIT (FAIL_DISASM);

  disasm_line *lines = malloc (sizeof (disasm_line) * count);
  uint32_t j = 0;
  for (; j < count; j++)
    {
      lines[j].addr = insn[j].address;
      snprintf (lines[j].disasm_code, DISASM_CODE_LEN, "%s %s",
                insn[j].mnemonic, insn[j].op_str);
    }

  cs_close (&handle);
  cs_free (insn, count);
  return 0;
}

void
free_disasm (disasm_line *line)
{
  free (line);
}
