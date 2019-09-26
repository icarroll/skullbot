def lw(w,p):
    return w.lower(), p
commons = dict(lw(*line.split()) for line in open("common_5000.txt").readlines()
               if " " in line)

extras = {
    "netherfield": "nETrfild",
    "michaelmas": "mIklms",
    "pemberley": "pEmbrl`",
    "lambton": "lAmtn",
    "curricle": "kUrIkl",
    "acquainting": "`kw!antN",
    "newlyborn": "nulibrn",
    "discomposure": "dskmpoZr",
    "overscrupulous": "`ovrskrpyls",
    "goodhumoured": "gUdhymrd",
    "threeandtwenty": "Tri` & twnt`",
    "resentfully": "rsntfl`",
    "fretfully": "frEtfl`",
    "fortnight's": "fortnts",
    "neices": "niss",
    "raptures": "rptSrs",
    "conjecturing": "kndZktSyrN",
    "bennet's": "bEnIts",
    "bennets": "bEnIts",
    "barefaced": "bErfsd",
    "favourable": "f!evErObOl",
    "bingley's": "bINgliz",
    "disconcerted": "dsknsrtd",
    "hertfordshire": "hrtfrdSr",
    "goodlooking": "gUdlkN",
    "gentlemanlike": "dZntlmnlk",
    "brotherinlaw": "brODErInlo`",
    "handsomer": "hAnsOmr",
    "unreserved": "`nrzrvd",
    "longbourn": "laNborn",
    "hurst's": "hUrsts",
    "fancying": "fAnsyN",
    "setdowns": "sEtdns",
    "humoured": "hyumErd",
    "censuring": "sEnSErN",
    "candour": "kAndEr",
    "pliancy": "pl!aOnsi`",
    "unassailed": "`OnOs!eld",
    "meanly": "minli`",
    "easiness": "`izins",
    "darcy's": "darsiz",
    "meryton": "merItn",
    "st": "s!ent",
    "lucases": "lukOsz",
    "overhearings": "`ovrhrNz",
    "lizzy's": "lIziz",
    "foxhounds": "fkshndz",
}

abbrevs = ["Mr.", "Mrs.", "St."]

def xlat1(c):
    if c.startswith("AA"): return "a"
    if c.startswith("AE"): return "A"
    if c.startswith("AH"): return "O"
    if c.startswith("AO"): return "o" #??? /a/?
    if c.startswith("AW"): return "^a"
    if c.startswith("AX"): return "O"   # I can't believe it's not schwa!
    if c.startswith("AXR"): return "Or"
    if c.startswith("AY"): return "!a"
    if c.startswith("EH"): return "E"
    if c.startswith("ER"): return "Er"
    if c.startswith("EY"): return "!e"
    if c.startswith("IH"): return "I"
    if c.startswith("IX"): return "I"
    if c.startswith("IY"): return "i"
    if c.startswith("OW"): return "^o"
    if c.startswith("OY"): return "!o"
    if c.startswith("UH"): return "U"
    if c.startswith("UW"): return "u"
    if c.startswith("UX"): return "u"

    if c == "B": return "b"
    if c == "CH": return "tS"
    if c == "D": return "d"
    if c == "DH": return "D"
    if c == "DX": return "t"   # flapped D
    if c == "EL": return "l"
    if c == "EM": return "m"
    if c == "EN": return "n"
    if c == "F": return "f"
    if c == "G": return "g"
    if c == "HH" or c == "H": return "h"
    if c == "JH": return "dZ"
    if c == "K": return "k"
    if c == "L": return "l"
    if c == "M": return "m"
    if c == "N": return "n"
    if c == "NG": return "N"
    if c == "NX": return "n"   # flapped N?
    if c == "P": return "p"
    if c == "Q": return "X"   # glottal stop, unused in cmudict
    if c == "R": return "r"
    if c == "S": return "s"
    if c == "SH": return "S"
    if c == "T": return "t"
    if c == "TH": return "T"
    if c == "V": return "v"
    if c == "W": return "w"
    if c == "WH": return "w"
    if c == "Y": return "y"
    if c == "Z": return "z"
    if c == "ZH": return "Z"

    raise Exception("unknown code {!r}".format(c))

vowels = "aeiouAEIOU!^"

def xlat(arpabet):
    phonets = [xlat1(c) for c in arpabet.split()]
    phonetic = str.join("", phonets)
    if phonetic[0] in vowels: phonetic = "`" + phonetic
    if phonetic[-1] in vowels: phonetic = phonetic + "`"
    return phonetic

text = open("cmudict-0.7b").read()
lines = [line.strip() for line in text.splitlines()
         if line[0].isalpha() and "(" not in line and "." not in line]

with open("pronunciation.txt", "w") as p:
    for line in lines:
        word,arpabet = line.split(maxsplit=1)
        word = word.lower()
        if word in commons: phonetic = commons[word]
        else: phonetic = xlat(arpabet)
        p.write("{} {}\n".format(word, phonetic))
    for word,phonetic in extras.items():
        p.write("{} {}\n".format(word, phonetic))
