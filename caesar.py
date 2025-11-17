alphabet = {"a" : 0, "b" : 1, "c" : 2, "d" : 3, "e" : 4, "f" : 5, "g" : 6, "h" : 7, "i" : 8, "j" : 9, "k" : 10, "l" : 11, "m" : 12, "n" : 13, "o" : 14, "p" : 15, "q" : 16, "r" : 17, "s" : 18, "t" : 19, "u" : 20, "v" : 21, "w" : 22, "x" : 23, "y" : 24, "z" : 25}
inv_alphabet = {v: k for k, v in alphabet.items()}

def encrypt(input_key: int, input_message: str) -> str:
    result: list = []
    for character in input_message:
        char: str = character.lower()
        if char in alphabet:
            encrypted_int: int = (alphabet[char] + input_key) % 26
            encrypted_char: str = inv_alphabet[encrypted_int]
            if character.isupper():
                    encrypted_char = encrypted_char.upper()
            result.append(encrypted_char)
        else:
            result.append(character)
    return "".join(result)


if __name__ == "__main__":
    key: int = ""
    while not key.isdigit():
        key: int = input("Key: ")
    else:
        key = int(key)

    message = input("Message: ")

    print(encrypt(key, message))
