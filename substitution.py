def create_dict(input_key: str) -> dict:
    alphabet = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"]
    input_list = [char.lower() for char in input_key]
    key_dict = dict(zip(alphabet, input_list))
    return key_dict


def encrypt(key_dict: dict, input_message: str) -> str:
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


if __name__ == "__main__":
    key: str = input("Enter a 26 character key: ")
    while not check_key(key):
        key: str = input("Enter a 26 character key: ")

    message: str = input("Enter a message: ")

    key_dictionary = create_dict(key)
    print(encrypt(key_dictionary, message))
