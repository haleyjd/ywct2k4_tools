.gba
.relativeinclude on

.create "patched.gba", 0x8000000
.incbin "input.gba"

;==================================================================================================
; Bug fixes
;==================================================================================================
.include "axe_of_despair.asm"
.include "burning_land.asm"

.close
