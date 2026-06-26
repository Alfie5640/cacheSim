import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import seaborn as sns

mpl.rcParams['font.size'] = 8
mpl.rcParams['figure.dpi'] = 200

#LOAD DATA
df = pd.read_csv("results/results.csv")
print(df.head())
print(df.columns)

#EDA
print(df.shape)
print(df.dtypes)
print(df.describe())
print(df.isnull().sum())

split = df["workload"].str.extract(r'(\w+)_a(\d+)_p(\d+)')

df["pattern"] = split[0]
df["associativity"] = split[1].astype(int)
df["prefetch"] = split[2].astype(int)

print(df.head())

split = df["workload"].str.extract(r'(\w+)_a(\d+)_p(\d+)')

df["pattern"] = split[0]
df["associativity"] = split[1].astype(int)
df["prefetch"] = split[2].astype(int)

print(df.head())


#VISUALISATION AND ANALYSIS
fig, ax = plt.subplots(figsize=(6,4))

sns.barplot(
    data=df,
    x="pattern",
    y="hit_rate",
    hue="associativity",
    palette="deep",
    ax=ax
)

ax.set_ylabel("Hit Rate (%)")
ax.set_xlabel("Memory Access Pattern")
ax.set_title("Hit Rate by Workload and Associativity")

plt.tight_layout()
plt.savefig("results/hitrate_workload.png")
plt.close()

fig, ax = plt.subplots(figsize=(6,4))

sns.lineplot(
    data=df,
    x="prefetch",
    y="hit_rate",
    hue="associativity",
    style="pattern",
    marker="o",
    palette="deep",
    ax=ax
)

ax.set_xlabel("Prefetch Distance")
ax.set_ylabel("Hit Rate (%)")
ax.set_title("Effect of Prefetch Distance on Cache Hit Rate")

plt.tight_layout()
plt.savefig("results/prefetch_hitrate.png")
plt.close()

fig, ax = plt.subplots(figsize=(6,4))

sns.barplot(
    data=df,
    x="associativity",
    y="conflict",
    hue="pattern",
    palette="deep",
    ax=ax
)

ax.set_ylabel("Conflict Misses")
ax.set_title("Conflict Misses by Associativity")

plt.tight_layout()
plt.savefig("results/conflict.png")
plt.close()

df["prefetch_efficiency"] = (
    100 * df["prefetch_useful"] /
    df["prefetch_total"].replace(0, pd.NA)
)

fig, ax = plt.subplots(figsize=(6,4))

sns.lineplot(
    data=df[df["prefetch"] > 0],
    x="prefetch",
    y="prefetch_efficiency",
    hue="pattern",
    marker="o",
    palette="deep",
    ax=ax
)

ax.set_ylabel("Useful Prefetches (%)")
ax.set_xlabel("Prefetch Distance")
ax.set_title("Prefetch Efficiency")

plt.tight_layout()
plt.savefig("results/prefetch_efficiency.png")
plt.close()

fig, ax = plt.subplots(figsize=(6,4))

sns.lineplot(
    data=df[df["prefetch"] > 0],
    x="prefetch",
    y="prefetch_pollution",
    hue="pattern",
    marker="o",
    palette="deep",
    ax=ax
)

ax.set_ylabel("Cache Pollution")
ax.set_xlabel("Prefetch Distance")
ax.set_title("Prefetch Pollution")

plt.tight_layout()
plt.savefig("results/pollution.png")
plt.close()

cols = [
    "hits",
    "misses",
    "conflict",
    "capacity",
    "hit_rate",
    "prefetch_total",
    "prefetch_useful",
    "prefetch_pollution"
]

fig, ax = plt.subplots(figsize=(6,4))

corr = df[cols].corr()

sns.heatmap(
    corr,
    annot=True,
    cmap="coolwarm",
    fmt=".2f",
    linewidths=0.5,
    ax=ax
)

ax.set_title("Correlation Matrix")

plt.tight_layout()
plt.savefig("results/heatmap.png")
plt.close()

summary = df.groupby(["pattern", "associativity"])[
    ["hit_rate", "conflict", "capacity"]
].mean()

print(summary)