import sys
import getopt


def get_args():
    text = ""
    code = 0
    text = sys.stdin.read()
    code = sys.argv[1]
    return text, code

def check_args(code):
    code_ok = False

    try:
        code = int(code)
        if 1 <= code <= 25:
            code_ok = True
    except ValueError:
            code_ok = False
     
        
    return code_ok

def mayus(text):
    text = text.upper()
    return text

def encode_chars(text, code):
    """
    Encode the given text by a Caesar cipher.
    Characters are interchanged with others according to their position in the alphabet and a
    distance code.
    The spaces are mantained
    """
    l_aux = []
    for c in text:
        new_c = ord(c) + code
        if (ord(c) < 65) or (90 < ord(c)):
            new_c = ord(c) 
        elif (new_c > 90):
            dif = new_c - 90
            new_c = 64 + dif
        c = chr(new_c)
        l_aux.append(c)
    new_text = "".join(l_aux)
    return new_text         
    
if __name__ == "__main__":
    text, code = get_args()
    check_args(code)
    code = int(code)
    text = mayus(text)
    text = encode_chars(text, code)
    print("%s" %(text))
    