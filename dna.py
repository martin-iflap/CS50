import csv
import sys

MEM = "mem.csv"

class DNASubject:
    def __init__(self, name: str, dna_str_int: dict):
        """Initialize a DNASubject with a name and DNA sequence counts"""
        self.name = name
        self.dna_str_int = dna_str_int

def get_dna_input() -> str:
    """Prompt the user to input a DNA sequence and validate that it contains only valid nucleotides"""
    valid_nucleotides = {"A","C","G","T"}
    while True:
        if nucleotide not in valid_nucleotides:
            dna = input("Enter a DNA sequence: ").upper()
            if all(n in valid_nucleotides for n in dna):
                return dna
            print("Invalid DNA sequence. Please enter a sequence containing only A, C, G, and T.")

def max_consecutive_repeats(dna: str, sequences: list[str]) -> dict:
    """Return the maximum number of consecutive repeats of sub in dna for each sub in sequences"""
    sub_count: dict = {}
    if not sequences:
        return {}
    for sub in sequences:
        max_count = 0
        i = 0
        step = len(sub)
        while i <= len(dna) - step:
            if dna[i:i+step] == sub:
                count = 0
                while dna[i:i+step] == sub:
                    count += 1
                    i += step
                if count > max_count:
                    max_count = count
            else:
                i += 1
        sub_count[sub] = max_count
    return sub_count

def create_subjects() -> tuple[list[DNASubject], list[str]]:
    """Create a list of DNASubject objects from the database CSV file"""
    subject_list: list[DNASubject] = []
    try:
        with open(MEM, "r", newline="") as f:
            reader = csv.reader(f, delimiter="|")
            sequences: list = next(reader)[1:]
            for row in reader:
                name = row[0] if len(row) > 0 else "Unknown"
                counts = [int(count) for count in row[1:]]
                dna_str_int: dict = dict(zip(sequences, counts))
                subject = DNASubject(name, dna_str_int)
                subject_list.append(subject)
    except FileNotFoundError:
        print(f"Error: The file {MEM} was not found.")
    return subject_list, sequences

def find_subject(dna_counts: dict, subjects: list[DNASubject], sequences: list[str]) -> None:
    """Find the subject that matches the given DNA input sequence and print the result if none match is found print Unknown"""
    if not sequences:
        print("No STR sequences to compare, database header is empty.")
    for subject in subjects:
        if all(subject.dna_str_int.get(seq, 0) == dna_counts.get(seq, 0) for seq in sequences):
            if subject.name == "Unknown":
                print("Match found for subject with no name")
            else:
                print(f"Match found: {subject.name}")
            return
    print("No match found")


if __name__ == "__main__":
    dna_input = "AGAT" * 28 + "AATG" * 42 + "TATC" * 14
    subject_l, sequences_l = create_subjects()
    if not sequences_l:
        print("No STR sequences found in the database.")
        sys.exit(1)
    dna_counts_l = max_consecutive_repeats(dna_input, sequences_l)
    find_subject(dna_counts_l, subject_l, sequences_l)
