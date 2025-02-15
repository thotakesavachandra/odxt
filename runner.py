import os
import pandas as pd

bucket_sizes = [5, 10, 15]
hamming_weights = [2, 3, 4, 5, 6, 7, 8, 9, 10]
# bucket_sizes = [15]
# hamming_weights = [4]
num_queries = 500


def read_stats(filename:str):
    lines = []
    with open(filename) as file:
        lines = file.readlines()
    lines = [elem[0:len(elem)-2] for elem in lines]
    return lines

def custom_read(filename:str):
    lines = []
    with open(filename) as file:
        lines = file.readlines()
    return lines


def func(bs:int, hw:int, to_setup:bool):
    dir_name = f"{bs}_{hw}"
    if(to_setup):
        os.system(f"make clean_all")
    else:
        os.system(f"make clean")
    os.system(f"make single_thread")
    os.system(f"./sse_sing_thread_out {dir_name}")
    os.system(f"make clean")

    # exp = custom_read(f"./test_vectors/{dir_name}/metaquery_result.csv")
    # act = custom_read(f"./results/{dir_name}/res_id.csv")
    # if act != exp:
    #     print(f"Error in {dir_name}")
    #     exit(1)

    s1 = read_stats(f"./test_vectors/{dir_name}/stats.csv")
    s2 = read_stats(f"./results/{dir_name}/res_stats.csv")
    for elem in s2:
        s1.append(elem)
    # s1.append(0); s1.append(0)
    return s1


results = []

for bs in bucket_sizes:
    for hw in hamming_weights:
        r = [bs, hw]
        for elem in func(bs, hw, hw==hamming_weights[0]):
            r.append(elem)
        results.append(r)
        

indexes = ["Bucket Size", "Hamming Weight", "Raw DB size", "Meta DB size", "#mkws in query", "Avg Precision", "Total Setup Time (microseconds)", "Total Query Time (microseconds)", "Average Query Time (microseconds)", "Average Practical Precision"]

df = pd.DataFrame(results, columns=indexes)
df.to_csv("results.csv", index=False)

print(df)