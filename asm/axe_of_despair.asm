;==================================================================================================
; Bug fix - Axe of Despair
;==================================================================================================

; Fix for Axe of Despair referencing invalid card ID from WWE:SDD when sent to GY
; Replaces:
;   dcd     0x12F
.org 0x80424CC
.area 4
    dcd     0x10D6
.endarea
