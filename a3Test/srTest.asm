; Check Jumps on Carry
org $100
mov #0, SR
mov #$1000, PC

; C = 1
org $1000
carryset mov #$1, SR
jnc carryset
mov #$1100, PC

; Z = 1
org $1100
zeroset mov #$2, SR
jnz zeroset
mov #$1200, PC

; N = 0
org $1200
negclr mov #0, SR
jn negclr
mov #$1300, PC

; V = 1, N = 1
org $1300
vset_nset mov #$104, SR
jl vset_nset
mov #$1400, PC

; V = 0, N = 1
org $1400
vclr_nset mov #$4, SR
jge vclr_nset
mov #$1500, PC

; V = 1, N = 0
org $1500
vset_nclr mov #$100, SR
jge vset_nclr

DONE word 0

END
