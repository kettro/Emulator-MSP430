; Cache Read Test

; Loops through a series of locations, to fill up the cache
; Demonstrates the addressing modes:
;   - Immediate
;   - Register Direct
;   - Register Indirect
;   - Register Indirect-AA
;   - Relative
;   - Absolute
;Demonstrates the types:
;   - type 1 (swpb)
;   - type 2 (mov, add, cmp, sub
;   - jumps (jne)

; Shows how the cache fills, and replaces its contents
; In the loop, the function fills the cache
; with locations that will fill each slot
; of the cache. As the cache fills, instructions will
; be found as hits, and returned from the cache.

org $1000
mov #$6000, r8
mov p, r9
loop swpb @r8+
cmp @r8, r9
jne loop
mov #finished, r15
sub &a, r15
mov r15,PC

org $1f00
ActuallyDone word 0
org $2000
finished word 0

org $6000
a word 1
b word 2
c word 3
d word 4
e word 5
f word 6
g word 7
h word 8
i word 9
j word 10
k word 11
l word 12
m word 13
n word 14
o word 15
p word 16
