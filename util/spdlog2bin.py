# converts input spdlog ASCII hex file to binary. 
# note---does NOT convert ASCII to its binary representation. Interprets ASCII characters as bytes.
# so assumes only '0'-'F' are present.
# looks for [trace] log level indicator in spdlog-generated log files and preserves data from those fields.

import sys

if __name__ == "__main__":
    from_file_name = sys.argv[1]
    to_file_name = sys.argv[2]
    print("reading from " + str(from_file_name))
    print("writing to " + str(to_file_name))

    hex_token = "[trace]"
    result = bytearray();
    block_count = 0;

    with open(from_file_name, "r") as from_file:
        lines = from_file.readlines()
        for line in lines:
            token_pos = line.find(hex_token)
            if token_pos >= 0:
                block_count += 1
                print("parsing block " + str(block_count))
                content = line[token_pos + len(hex_token) + 1:]
                result.extend(bytearray.fromhex(content))

        print("parsed " + str(block_count) + " blocks")
        with open(to_file_name, "wb") as to_file:
            to_file.write(result)
    
    