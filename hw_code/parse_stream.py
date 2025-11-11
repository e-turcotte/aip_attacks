def parse_letter_sequence(data):
    """
    Parses a string of letter data, mapping each unique letter to a unique number.
    
    Args:
        data (str): The multi-line string containing the data.

    Returns:
        None. Prints the results directly.
    """
    
    letter_to_num_map = {}
    numbered_sequence = []
    next_unique_id = 0 # Start counting from 1

    # Split the raw data into individual lines
    lines = data.strip().split('\n')

    for line in lines:
        # Clean up any potential whitespace
        line = line.strip()
        if not line:
            continue
        if "//" in line:
            continue
        # Split the line into parts. Handles "1 T" and "1P"
        if len(line) > 1 and not ' ' in line:
             # Handle cases like "1P"
             parts = [line[0], line[1:]]
        else:
             parts = line.split()

        # Ensure we have at least two parts (like '1' and 'T')
        if len(parts) < 2:
            continue
            
        letter = parts[1]

        # Check if we've seen this letter before
        if letter not in letter_to_num_map:
            # If not, assign it the next available unique number
            letter_to_num_map[letter] = next_unique_id
            next_unique_id += 1
            
        # Add the letter's assigned number to our sequence
        numbered_sequence.append(letter_to_num_map[letter])

    # --- Print the results ---

    print("--- Letter to Number Map ---")
    print(letter_to_num_map)
    print("\n")

    print("--- Total Unique Numbers ---")
    # The total is the number of entries in our map
    total_unique = len(letter_to_num_map)
    print(total_unique)
    print("\n")

    print("--- Number Sequence ---")
    print(numbered_sequence)

# Run the parser function
if __name__ == "__main__":
    trace_file = "trace.txt"
    try:
        with open(trace_file, 'r') as f:
            file_content = f.read()
        parse_letter_sequence(file_content)
        
    except FileNotFoundError:
        print(f"Error: The file '{trace_file}' was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")