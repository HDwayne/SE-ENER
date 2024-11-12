import random

# Tri a bulle sans affichage
def sort_bubble(l):
    hasChanged = True
    while hasChanged:
        hasChanged = False

        for i in range(len(l)-1):
            if l[i] > l[i+1]:
                l[i], l[i+1] = l[i+1], l[i]
                hasChanged = True

if __name__ == "__main__":
    l = random.sample(range(0, 1000), 50)
    print(l)
    sort_bubble(l)
    print(l)