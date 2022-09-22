import os
# Lots of reference to this https://github.com/microsoft/uf2

content = bytearray(open("pico.uf2", 'rb').read())
fsize = os.path.getsize("pico.uf2")
blocksnum = fsize//512
print("Number of blocks: " + hex(blocksnum))

# Ok so first we go through each block of 512 bytes and increment the "Total number of blocks in file" by one
newnlocksnum = blocksnum + 1
for i in range(blocksnum):
    content[(i*512)+24] = newnlocksnum & 0xFF
    content[(i*512)+25] = (newnlocksnum >> 8) & 0xFF
    content[(i*512)+26] = (newnlocksnum >> 16) & 0xFF
    content[(i*512)+27] = (newnlocksnum >> 24) & 0xFF

# Next up we seperate the last 512 bytes
# We can then edit these to the new stuff we want to add and then add them back after
newbytes = content[-512:]

# Edit the Sequential block number attribute
newbytes[20] = blocksnum & 0xFF
newbytes[21] = (blocksnum >> 8) & 0xFF
newbytes[22] = (blocksnum >> 16) & 0xFF
newbytes[23] = (blocksnum >> 24) & 0xFF


# Set where the data should go (one after the end of the program
address = 0x10000000 + (blocksnum << 8)
newbytes[12] = address & 0xFF
newbytes[13] = (address >> 8) & 0xFF
newbytes[14] = (address >> 16) & 0xFF
newbytes[15] = (address >> 24) & 0xFF

# Clear the data section
for i in range(476):
    newbytes[i+0x20] = 0
newbytes[0x20] = 0x65

# Add our string
#str = "Hello World!".encode()
#for i in range(len(str)):
#    newbytes[0x20+i] = str[i]


content += newbytes
# Save final file
with open("pico2.uf2", 'wb') as f:
    f.truncate(0)
    f.write(content)