```
@file: plot_hpc.jl
@description: Plot the measurements of the MPI experiments conducted
on the HPC platform.
```

using PlotlyJS
using CSV, DataFrames

# Load the measurements.
results2 = DataFrame(CSV.File("hpc/results2.txt", header = 0));
results4 = DataFrame(CSV.File("hpc/results4.txt", header = 0));
results8 = DataFrame(CSV.File("hpc/results8.txt", header = 0));
results16 = DataFrame(CSV.File("hpc/results16.txt", header = 0));
results32 = DataFrame(CSV.File("hpc/results32.txt", header = 0));
results64 = DataFrame(CSV.File("hpc/results64.txt", header = 0));

# Keep them all in a DataFrame, take the size.
hpc_results = DataFrame(
    results2 = results2[!, 1],
    results4 = results4[!, 1],
    results8 = results8[!, 1],
    results16 = results16[!, 1],
    results32 = results32[!, 1],
    results64 = results64[!, 1],
);

measurements = size(hpc_results)[2];
rlplots = Vector{GenericTrace}(undef, measurements);

rounds = 1:10;
names = [string(2^i) * " processes" for i in 1:measurements];

# Make the scatters.
for i = 1:measurements
    rlplots[i] = scatter(
        y = hpc_results[!, i],
        x = rounds,
        marker_size = 8,
        name = names[i]
    );
end

# Put them all in a plot.
rl = plot(rlplots);
rl.plot.layout["title"] = "HPC experiments"
savefig(rl, "../output/hpc.jpeg", width = 872, height = 654);
