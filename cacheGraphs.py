import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

print(df)

plt.bar(df["workload"], df["hit_rate"])
plt.ylabel("Hit Rate (%)")
plt.title("Cache Hit Rate by Workload")

plt.show()