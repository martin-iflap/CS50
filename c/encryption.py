alphabet_caesar = {"a" : 0, "b" : 1, "c" : 2, "d" : 3, "e" : 4, "f" : 5, "g" : 6, "h" : 7, "i" : 8, "j" : 9, "k" : 10, "l" : 11, "m" : 12, "n" : 13, "o" : 14, "p" : 15, "q" : 16, "r" : 17, "s" : 18, "t" : 19, "u" : 20, "v" : 21, "w" : 22, "x" : 23, "y" : 24, "z" : 25}
inv_alphabet_caesar = {v: k for k, v in alphabet_caesar.items()}

def encrypt_caesar(input_key: int, input_message: str) -> str:
    result: list = []
    for character in input_message:
        char: str = character.lower()
        if char in alphabet_caesar:
            encrypted_int: int = (alphabet_caesar[char] + input_key) % 26
            encrypted_char: str = inv_alphabet_caesar[encrypted_int]
            if character.isupper():
                    encrypted_char = encrypted_char.upper()
            result.append(encrypted_char)
        else:
            result.append(character)
    return "".join(result)

def caesar_cipher():
    key: int = ""
    while not key.isdigit():
        key: int = input("Key (number): ")
    else:
        key = int(key)
    message = input("Message: ")
    print(encrypt_caesar(key, message))

 # --------------------- Substitution Cipher ---------------------

def create_dict(input_key: str) -> dict:
    alphabet_s = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"]
    input_list = [char.lower() for char in input_key]
    key_dict = dict(zip(alphabet_s, input_list))
    return key_dict

def encrypt_substitution(key_dict: dict, input_message: str) -> str:
    result: list = []
    for character in input_message:
        char: str = character.lower()
        if char in key_dict.keys():
            encrypted_char: str = key_dict[char]
            if character.isupper():
                encrypted_char = encrypted_char.upper()
            result.append(encrypted_char)
        else:
            result.append(character)
    return "".join(result)

def check_key(input_key: str) -> bool:
    for char in input_key:
        count = input_key.lower().count(char)
        if count > 1:
            return False
    if len(input_key) == 26:
        return True
    else: return False

def substitution_cipher():
    key: str = input("Enter a 26 character key: ")
    while not check_key(key):
        key: str = input("Enter a 26 character key: ")
    message: str = input("Enter a message: ")
    key_dictionary = create_dict(key)
    print(encrypt_substitution(key_dictionary, message))

def main():
    cipher_type: int = input("Choose cipher caesar(1) or substitution(2): ")
    while not cipher_type.isdigit() or int(cipher_type) not in [1, 2]:
        cipher_type: int = input("Choose cipher caesar(1) or substitution(2): ")
    cipher_type = int(cipher_type)
    if cipher_type == 1:
        caesar_cipher()
    elif cipher_type == 2:
        substitution_cipher()
    else:
        print("ERROR occurred")

if __name__ == "__main__":
    main()
