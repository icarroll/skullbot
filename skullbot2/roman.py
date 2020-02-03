roman_numerals = dict(I=1, V=5, X=10, L=50, C=100, D=500, M=1000)

def decode_roman_numeral(roman):
    n = 0
    most = -1
    for c in reversed(roman):
        cur = roman_numerals[c]
        if cur >= most:
            n += cur
            most = cur
        else:
            n -= cur
    return n
