import random

# Tri par tas sans affichage
def sort_heapsort(l):
    n = len(l)
    # Il n'y a que n // 2 racine dans le tas, le reste sont des feuilles
    for i in range(n // 2 - 1, -1, -1):
        restore_heap_properties(l, n, i)
    for i in range(n-1, 0, -1):
        l[i], l[0] = l[0], l[i]
        restore_heap_properties(l, i, 0)

# Fonction qui permet de reformer un tas
def restore_heap_properties(l, n, i):
    maxVal = i
    left = 2 * i + 1
    right = 2 * i + 2

    if left < n and l[maxVal] < l[left]:
        maxVal = left

    if right < n and l[maxVal] < l[right]:
        maxVal = right

    if maxVal != i:
        l[i], l[maxVal] = l[maxVal], l[i]
        restore_heap_properties(l, n, maxVal)

if __name__ == "__main__":
    l = random.sample(range(0, 1000), 50)
    print(l)
    generator = sort_heapsort(l)
    print(l)