import random

# Tri par sélection itératif
def sort_selection(l):
    for i in range(len(l)-1):
        minInd = i
        for j in range(i+1, len(l)):
            if l[j] < l[minInd]:
                minInd = j
        if i != minInd:
            l[i], l[minInd] = l[minInd], l[i]

# Tri par sélection récursif
def sort_selection_rec(l, start=0):
    if len(l)-1 > start:
        minInd = start
        for j in range(start + 1, len(l)):
            if l[j] < l[minInd]:
                minInd = j
        if start != minInd:
            l[start], l[minInd] = l[minInd], l[start]
        sort_selection_rec(l, start + 1)

if __name__ == "__main__":
    l = random.sample(range(0, 1000), 50)
    print(l)
    sort_selection_rec(l)
    print(l)