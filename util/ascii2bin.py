# converts input ASCII file to binary. 
# note---does NOT convert ASCII to its binary representation. Interprets ASCII characters as bytes.
# so assumes only '0'-'F' are present.

import sys

if __name__ == "__main__":
    from_file_name = sys.argv[1]
    to_file_name = sys.argv[2]
    print("reading from " + str(from_file_name))
    print("writing to " + str(to_file_name))

    with open(from_file_name, "r") as from_file:
        content = from_file.read()
        result = bytearray.fromhex(content)

        with open(to_file_name, "wb") as to_file:
            to_file.write(result)
    