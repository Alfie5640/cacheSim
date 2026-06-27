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

df["prefetch_efficiency"] = (
    100 * df["prefetch_useful"] /
    df["prefetch_total"].replace(0, pd.NA)
)

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
fig, ax1 = plt.subplots(figsize=(6,4))

# Filter
mask = (df["pattern"] == "sequential") & (df["associativity"] == 4) & (df["prefetch"] > 0)
plot_df = df[mask].copy()

ax1.plot(plot_df["prefetch"], plot_df["prefetch_efficiency"], 
         color="steelblue", marker="o", label="Useful (%)")
ax1.set_xlabel("Prefetch Distance")
ax1.set_ylabel("Useful Prefetches (%)", color="steelblue")
ax1.tick_params(axis="y", labelcolor="steelblue")

ax2 = ax1.twinx()
pollution_pct = 100 * plot_df["prefetch_pollution"] / plot_df["prefetch_total"]
ax2.plot(plot_df["prefetch"], pollution_pct,
         color="tomato", marker="s", linestyle="--", label="Pollution (%)")
ax2.set_ylabel("Pollution (% of prefetches)", color="tomato")
ax2.tick_params(axis="y", labelcolor="tomato")

lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, loc="center right")

ax1.set_title("Prefetch Usefulness vs Pollution (4-way Sequential)")
plt.tight_layout()
plt.savefig("results/prefetch_tradeoff.png")
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