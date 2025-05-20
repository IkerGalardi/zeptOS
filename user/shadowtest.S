.global main
main:
    li ra, 0xdeadbeef
    sspush ra
    li ra, 0x0
    sspopchk ra
