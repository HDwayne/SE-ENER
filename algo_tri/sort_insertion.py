import random

# Tri par insertion sans affichage
def sort_insertion(l):
    for i in range(len(l)):
        j = i
        while j > 0 and l[j-1] > l [j]:
            l[j-1], l[j] = l[j], l[j-1]
            j -= 1

if __name__ == "__main__":
    l = random.sample(range(0, 1000), 50)
    print(l)
    sort_insertion(l)
    print(l)