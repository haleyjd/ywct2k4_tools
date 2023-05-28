;==================================================================================================
; Bug fix - Burning Land
;==================================================================================================

; Fix for Burning Land applying damage to wrong player during opponent's standby phase
; Replaces:
;   add     r0, r7, #0
.org 0x80699D6
.area 2
    add     r0, r6, #0
.endarea
