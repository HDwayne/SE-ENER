import random

#Fonction de partitionnement du tableau, le pivot est le dernier élément
def partition(l, start, end):
    piv = l[end]
    j = start
    for i in range(start,end):
        if l[i] <= piv:
            l[i], l[j] = l[j], l[i]
            j += 1
    l[j], l[end] = l[end], l[j]
    return j

# Tri rapide sans affichage
def sort_quicksort(l, start=0, end=None):
    if end == None:
        end = len(l)-1

    if end > start:
        pivot = partition(l, start, end)
        sort_quicksort(l, start, pivot-1)
        sort_quicksort(l, pivot+1, end)


if __name__ == "__main__":
    l = random.sample(range(0, 1000), 50)
    print(l)
    sort_quicksort(l)
    print(l)