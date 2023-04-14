import sys
import pdb


ALPHABET = {"A" :0.08167, "B" :0.01492, "C" :0.02782, "D" :0.04253, "E" :0.12702,
            "F" :0.02228, "G" :0.02015, "H" :0.06094, "I" :0.06966, "J" :0.00153,
            "K" :0.00772, "L" :0.04025, "M" :0.02406, "N" :0.06749, "O" :0.07507,
            "P" :0.01929, "Q" :0.00095, "R" :0.05987, "S" :0.06327, "T" :0.09056,
            "U" :0.02758, "V" :0.00978, "W" :0.02360, "X" :0.00150, "Y" :0.01974,
            "Z" :0.00074}

GRAM_TYPES = {
    "di": ['TH', 'HE', 'IN', 'EN', 'NT', 'RE', 'ER', 'AN', 'TI', 'ES', 'ON', 'AT', 'SE', 'ND',
           'OR', 'AR', 'AL', 'TE', 'CO', 'DE', 'TO', 'RA', 'ET', 'ED', 'IT', 'SA', 'EM', 'RO'],
    "tri": ['THE', 'AND', 'THA', 'ENT', 'ING', 'ION', 'TIO', 'FOR',
            'NDE', 'HAS', 'NCE', 'EDT', 'TIS', 'OFT', 'STH', 'MEN']
}

MAX_CANDIDATES = 1

MAX_CODES = 25

def sort_dict_values(dictionary: dict) -> list:
    dict_values = list(dictionary.values())
    sorted_values = sorted(dict_values,reverse= True)
    return sorted_values


def sort_frequencies(freqs_alphabet):
    sorted_alphabet = {}
    list_letters = freqs_alphabet.keys()
    sorted_letters = sorted(list_letters)
    for letter in sorted_letters:
        sorted_alphabet[letter] = freqs_alphabet[letter]

    return sorted_alphabet


def get_frequency(text):
    """
    Get the frequency of each letter in the text then sort them and append it like {"A": freq} 
    """
    length = len(text)
    freqs_alphabet = {}
    for c in text:
        if not (any(c in letter for letter in freqs_alphabet)):#Checks if the char was appended previously 
            counter = text.count(c)
            freq = round(counter/length, 5)
            freqs_alphabet[c] = freq
    freqs_alphabet = sort_frequencies(freqs_alphabet)
    return freqs_alphabet

def get_caesar(text, code):
    """
    Decode the given text by a Caesar decipher.
    Characters are interchanged with others according to their position in the alphabet and a
    distance code.
    The spaces are mantained
    """
    l_aux = []
    for c in text:
        new_c = ord(c) - code
        if (ord(c) < 65) or (90 < ord(c)):
            new_c = ord(c) 
        elif (new_c < 65):
            dif = 65 - new_c
            new_c = 91 - dif
        c = chr(new_c)
        l_aux.append(c)
    new_text = "".join(l_aux)
    return new_text   


def get_freqs_codes(text):
    """
    Get all possibles decoded texts and obtains the frequency of the alphabet in each decoded text
    depending on the code (number betwen 1-25) like {code: freq_alphabet}, freq_alphabet is in form
    of {"A":freq}
    """
    freqs_codes = {}
    for i in range(MAX_CODES+1):
        caesar_text = get_caesar(text, i)
        aux_text = caesar_text.replace(" ", "")
        freq_alphabet = get_frequency(aux_text) 
        freqs_codes[i] = freq_alphabet

    return freqs_codes

def replace_tabs(text):
    while ("  " in text):
        text = text.replace("  ", " ")
    return text

def get_caesar_texts(text):
    """
    Obtains all the different caesar texts 
    """
    caesar_texts = {}
    text = replace_tabs(text)
    for i in range(MAX_CODES+1):
        caesar = get_caesar(text, i)
        caesar_texts[i] = caesar
    return caesar_texts

def get_manhattan(a, b):
    """"Get the distance between two lists using the manhattan method"""
    value = 0
    for i in range(len(b)):
        value += abs(a[i]-b[i])
    value = value/len(b)
    
    return value


def get_distance(freqs_codes):
    """
    Obtains the distance between the original text and all the 25 different caesar texts and collect them
    in a list
    """
    list_distances = []
    abcs_freqs = list(ALPHABET.values())
    for i in range(MAX_CODES+1):
        list_freqs = list(freqs_codes[i].values())
        distance = get_manhattan(abcs_freqs, list_freqs)
        list_distances.append(distance)
        
    return list_distances

def choose_distance(distances):
    """
    Choose the candidates depending on the shorter distances between the general frequency of
    letters in english and the frequency of letters in all the possible caesar texts.
    @distances: Has the distances sorted by the caesar codes, {1-25} as {0-24}.
    @sorted_distances: Has the distances sorted form the sortest to the largest distance
    @codes: Is a list with all the caesar codes whose distance is repeated
    @dict_distances: Has the caesar code as key and his distance as value 
    """
    list_distances = []
    sorted_distances = sorted(distances)
    for i in range(MAX_CANDIDATES): 
        code = [index for index, x in enumerate(distances) if (x == sorted_distances[i]) and (not index in list_distances)]
        list_distances.append(code[0])
    return code[0]



def get_grams(caesar_dict, gram_type):
    
    """
    Get the bigrams or trigrams of an caesar text depenig on the most common bi/trigrams in english.
    @gram_type: Indicate what kind of gram is going to be evaluated
    @counter: A list with all the indexes which are the start of a bi/trigram, it's maded by looping
    through each char from the text and check if it starts with a bi/trigram with overlap
    @grams: Indicates if we want to search a bigram or a trigram 
    """
    caesar_grams = {}
    caesar_texts = list(caesar_dict.values())
    caesar_codes = list(caesar_dict.keys())
    
    gram_type = GRAM_TYPES[gram_type]
    
    for i in range(len(caesar_codes)):
        text = caesar_texts[i]
        num_digrams = 0
        for j in range(len(gram_type)):
            counter = [index for index in range(len(text)) if text.startswith(gram_type[j], index)]
            num_digrams += len(counter)
        caesar_grams[caesar_codes[i]] = num_digrams 
            
    return caesar_grams
    

def choose_grams(caesar_grams):
    """
    Choose the candidates depending on the amount of bi/trigrams sorting them to make it easier
    """
    sorted_grams = []
    ngrams = sort_dict_values(caesar_grams)
    
    for i in range(MAX_CANDIDATES):
        for key,val in caesar_grams.items():
            if (val == ngrams[i]) and (len(sorted_grams) < MAX_CANDIDATES): 
                #If there are few codes with the same value they are appended so we restrict with in len
                sorted_grams.append(key)
                code_grams = key
    return code_grams


def choose_candidates(distances, digrams, trigrams, texts):
    """
    Choose the final candidates depending on the relevance on their distance of letter frequencies
    and number of digrams and trigrams
    """
    
    if distances == digrams == trigrams:
        print(f"{distances}: {texts[distances]}\n")
    elif distances == digrams and distances != trigrams:
        print(f"{distances}: {texts[distances]}\n")
        print(f"{trigrams}: {texts[trigrams]}")
    elif trigrams == digrams and trigrams != distances:
        print(f"{trigrams}: {texts[trigrams]}\n")
        print(f"{distances}: {texts[distances]}")
    else: 
        print(f"{distances}: {texts[distances]}\n")
        print(f"{trigrams}: {texts[trigrams]}\n")
        print(f"{digrams}: {texts[digrams]}")
          

def get_result(text):
    freqs_codes = get_freqs_codes(text)
    caesar_texts = get_caesar_texts(text)
    
    list_distances = get_distance(freqs_codes)
    code_distance = choose_distance(list_distances)
    
    caesar_digrams = get_grams(caesar_texts, "di")
    caesar_trigrams = get_grams(caesar_texts, "tri")
    code_digrams = choose_grams(caesar_digrams)
    code_trigrams = choose_grams(caesar_trigrams)
    choose_candidates(code_distance, code_digrams, code_trigrams, caesar_texts)
    
        
        


if __name__ == "__main__":
    text  = sys.stdin.read()
    get_result(text)
