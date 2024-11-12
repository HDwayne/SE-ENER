import random
def sort_countsort(l):
    maxVal = max(l)
    output = [0 for i in range(maxVal+1)]

    for i in l:
        output[i] += 1

    ind = 0
    for i in range(len(output)):
        for j in range(output[i]):
            l[ind] = i
            ind +=1

if __name__ == "__main__":
    l = random.sample(range(0, 1000), 50)
    print(l)
    generator = sort_countsort(l)
    print(l)