def bit(sq):
    return 1 << sq

def get_full_rook_rays(sq):
    rank, file = divmod(sq, 8)
    mask = 0
    # North / South
    for r in range(8):
        if r != rank:
            mask |= bit(r * 8 + file)
    # East / West
    for f in range(8):
        if f != file:
            mask |= bit(rank * 8 + f)
    return mask

def get_full_bishop_rays(sq):
    rank, file = divmod(sq, 8)
    mask = 0
    for i in range(1, 8):
        # North-East
        if rank + i < 8 and file + i < 8: mask |= bit((rank + i) * 8 + (file + i))
        # North-West
        if rank + i < 8 and file - i >= 0: mask |= bit((rank + i) * 8 + (file - i))
        # South-East
        if rank - i >= 0 and file + i < 8: mask |= bit((rank - i) * 8 + (file + i))
        # South-West
        if rank - i >= 0 and file - i >= 0: mask |= bit((rank - i) * 8 + (file - i))
    return mask

def print_c_array(name, generator):
    print(f"const p_bitboard {name}[64] = {{")
    for sq in range(64):
        val = generator(sq)
        # Format as 16-character padded hex with ULL suffix
        if sq % 4 == 0:
            print("    ", end="")
        print(f"0x{val:016X}ULL,", end=" " if sq % 4 != 3 else "\n")
    print("};\n")

if __name__ == "__main__":
    print("#include <stdint.h>")
    print("typedef uint64_t p_bitboard;\n")
    
    print_c_array("FULL_ROOK_RAYS", get_full_rook_rays)
    print_c_array("FULL_BISHOP_RAYS", get_full_bishop_rays)

