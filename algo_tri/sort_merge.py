import random

# Tri fusion sans affichage
def sort_merge(l):
    n = len(l)

    if n > 1:
        mid = n // 2
        tab_left = l[0:mid]
        tab_right = l[mid:n]
        sort_merge(tab_left)
        sort_merge(tab_right)

        i = j = k = 0
        while i < len(tab_left) and j < len(tab_right):
            if tab_left[i] < tab_right[j]:
                l[k] = tab_left[i]
                i += 1
            else:
                l[k] = tab_right[j]
                j += 1
            k += 1

        while i < len(tab_left):
            l[k] = tab_left[i]
            i+=1
            k+=1

        while j < len(tab_right):
            l[k] = tab_right[j]
            j+=1
            k+=1

if __name__ == "__main__":
    l = random.sample(range(0, 1000), 500)
    print(l)
    sort_merge(l)
    print(l)