instructions = [3000 + i for i in range(-1, 5)]


with open("temp.txt", "wb") as file:
    for instruction in instructions:
        instr_bytes = instruction.to_bytes(2, "little")
        file.write(instr_bytes)

with open("temp.txt", "rb") as file:
    for byte in file.read():
        print(byte)
