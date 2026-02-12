# HelixForge Quick Start Guide

## 5-Minute Setup

### 1. Build
```bash
git clone https://github.com/yourusername/helixforge
cd helixforge
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### 2. Run First Command
```bash
# Create test sequence
echo ">test_gene" > test.fasta
echo "ATGGCCCCCAAGTCCAGTCCACAGCCGGCGAGGAGGGCTCGGTCACCACC" >> test.fasta

# Evaluate
./helixforge evaluate test.fasta --metrics gc_content
```

### 3. Optimize
```bash
./helixforge optimize test.fasta \
  --objective maximize_expression \
  --algorithm anneal \
  --iterations 1000 \
  --seed 42
```

## Common Use Cases

### Codon Optimization
```bash
helixforge optimize gene.fasta \
  --objective maximize_cai \
  --constraints "gc=0.50-0.60,avoid:GAATTC" \
  --algorithm genetic \
  --iterations 5000
```

### Mutation Study
```bash
# Generate 100 mutants
helixforge mutate gene.fasta --num 100 --seed 42 > mutants.fasta

# Scan all positions
helixforge scan gene.fasta --output scan.json
```

### Batch Processing
```bash
for file in sequences/*.fasta; do
    helixforge evaluate "$file" \
      --metrics gc_content,cai,rare_codons \
      --output "${file%.fasta}_metrics.json"
done
```

## Python Quick Start

```python
import helixforge as hf

# Load and optimize
seq = hf.Sequence.from_dna("ATGCGT...")
result = hf.optimize(seq, objective="expression", seed=42)
print(f"Improved by: {result.best_score:.2%}")
```

## Next Steps

- Read the full [README.md](README.md)
- Check [examples/](examples/) for more code
- See [docs/](docs/) for API reference

---

**Need help?** Open an issue on GitHub!
