---
annotations_creators:
  - found
language:
  - en
license: cc-by-4.0 # Or the specific license of the original GSE186433 data if different
multilinguality: monolingual
pretty_name: Human Cornea Atlas (snRNA-seq)
size_categories:
  - 10K<n<100K # Based on ~19K cells (19,472)
source_datasets:
  - original
tags:
  - single-cell
  - snRNA-seq
  - human
  - cornea
  - eye
  - vision
  - aging
  - cell-atlas
  - gene-expression
  - pca
  - umap
  - dimensionality-reduction
  - 10x-genomics
---

# Human Cornea Atlas (snRNA-seq) Dataset

## Dataset Overview

This dataset comprises single-nucleus RNA sequencing (snRNA-seq) data specifically focusing on the cellular heterogeneity of the **human cornea**. It provides a high-resolution view of various cell populations and their gene expression profiles across different layers of this critical ocular tissue.

The data was sourced from a research paper providing a comprehensive single-cell transcriptome atlas of the human cornea. This processed version has been transformed from raw 10x Genomics output files (multiple samples) into standardized `.parquet` formats, enhancing its usability for machine learning, bioinformatics pipelines, and enabling in-depth insights into corneal biology and potential age-related changes.

### Relevance to Aging and Longevity Research

The human cornea, a vital component for clear vision, undergoes structural and functional changes with age, contributing to conditions like corneal opacities and dry eye syndrome, which become more prevalent in older adults. Understanding the specific cell populations and their molecular profiles across the cornea can shed light on age-related cellular processes.

While the original study primarily focused on corneal heterogeneity, this dataset can be leveraged to:
* Investigate **age-related molecular shifts** within specific corneal cell types (e.g., epithelial, stromal keratocytes, endothelial cells), if age metadata is available for the donors.
* Uncover how cellular processes like extracellular matrix remodeling, stress response, and maintenance of transparency change with age.
* Identify potential **biomarkers** or **therapeutic targets** for age-related corneal pathologies.
* Explore the contribution of cellular aging to the overall health and function of the human eye.

This dataset thus serves as a valuable resource for understanding the intricate cellular mechanisms within the cornea, with potential implications for aging research and ocular health.

---

## Data Details

* **Organism:** *Homo sapiens* (Human)
* **Tissue:** Cornea (including epithelium, Bowman's layer, stroma, Descemet's membrane, endothelium, and limbus)
* **Cell Types:** Various corneal cell populations (e.g., epithelial cells, keratocytes, endothelial cells, limbal stem cells). *Specific cell type annotations should be found in `cell_metadata.parquet` (if available in original metadata).*
* **Technology:** 10x Genomics 3' snRNA-seq
* **Condition:** Healthy donor corneas across different individuals.
* **Number of Cells:** ~19,472 (combined from 12 samples after initial filtering)
* **Original Data Source:** GSE186433 from NCBI Gene Expression Omnibus (GEO).
    * **GEO Accession:** [GSE186433](https://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?acc=GSE186433)
    * **Original Files Used:** Multiple `GSM..._g..._barcodes.tsv.gz`, `_features.tsv.gz`, `_matrix.mtx.gz` files, and `GSE186433_metadata_percell.csv` (or `.csv.gz`).

---

## Dataset Structure

The dataset is provided in formats commonly used in single-cell genomics and tabular data analysis. After processing, the following files are generated:

* **`expression.parquet`**: A tabular representation of the gene expression matrix (normalized and log-transformed `adata.X`), where rows are cells and columns are genes. Ideal for direct use as input features in machine learning models.
* **`gene_metadata.parquet`**: A tabular representation of the gene (feature) metadata (`adata.var`), providing details about each gene.
* **`cell_metadata.parquet`**: Contains comprehensive metadata for each cell (`adata.obs`), including original sample ID and (if successfully merged from external source) donor information, age, sex, and cell type annotations. This is crucial for labeling and grouping cells in ML tasks.
* **`pca_embeddings.parquet`**: Contains the data after **Principal Component Analysis (PCA)**. This is a linear dimensionality reduction, where each row corresponds to a cell, and columns represent the principal components.
* **`pca_explained_variance.parquet`**: A table showing the proportion of variance explained by each principal component, useful for assessing the PCA's effectiveness.
* **`umap_embeddings.parquet`**: Contains the data after **UMAP (Uniform Manifold Approximation and Projection)**. This is a non-linear dimensionality reduction, providing 2D or 3D embeddings excellent for visualization and capturing complex cell relationships.
* **`highly_variable_gene_metadata.parquet`**: Metadata specifically for genes identified as highly variable across cells during preprocessing. These genes often capture the most biological signal and are commonly used for dimensionality reduction and feature selection.
* **`gene_statistics.parquet`**: Basic statistics per gene, such as mean expression (of log-normalized data) and the number of cells a gene is expressed in.

---

## Data Cleaning and Processing

The raw data was sourced from GSE186433. The processing steps, performed using a Python script, are designed to prepare the data for machine learning and in-depth bioinformatics analysis:

1.  **Raw 10x Data Loading & Concatenation:** Individual `matrix.mtx.gz`, `barcodes.tsv.gz`, and `features.tsv.gz` files for each sample were loaded using `scanpy.read_10x_mtx`. Cell barcodes were made unique across samples by prepending the sample ID (e.g., `GSM5651509_g001_AAACCTGAGCGGATTG-1`). All samples were then concatenated into a single AnnData object.
2.  **External Metadata Merging:** An external metadata CSV (`GSE186433_metadata_percell.csv`) was loaded and merged with the AnnData object's cell metadata (`adata.obs`). This involved constructing matching cell IDs by combining sample identifiers and raw barcodes. *(Note: During processing, if standard column names were not found or aligned incorrectly, some external metadata might not have been fully integrated. Please check `cell_metadata.parquet` for available columns and verify merge success.)*
3.  **Basic Quality Control (QC):** Cells with fewer than 200 genes and genes expressed in fewer than 3 cells were filtered out to remove low-quality data.
4.  **Normalization and Log-transformation:** Total counts per cell were normalized to a target sum of 10,000, and then the data was log-transformed.
5.  **Highly Variable Gene (HVG) Identification:** `scanpy.pp.highly_variable_genes` was used to identify a subset of highly variable genes (top 4000) that capture most of the biological signal. This subset is then used for efficient dimensionality reduction.
6.  **Principal Component Analysis (PCA):** Performed on the scaled highly variable gene expression data to generate `pca_embeddings.parquet` and `pca_explained_variance.parquet`.
7.  **UMAP Embeddings:** UMAP was performed on the PCA embeddings to generate `umap_embeddings.parquet`, providing a non-linear 2D representation for visualization.

---

## Usage

This dataset is ideal for a variety of research and machine learning tasks in the context of human corneal biology and its potential connections to aging:

### Single-Cell Analysis
Explore cellular heterogeneity, identify novel cell states, and characterize gene expression patterns within the human cornea.

### Aging & Ocular Health Research
* Investigate potential age-related molecular shifts within specific corneal cell types.
* Uncover how cellular processes related to corneal transparency, structure, and repair change with age.
* Identify novel markers for corneal cell populations and their alterations in aging or disease.

### Machine Learning
* **Clustering:** Apply clustering algorithms (e.g., K-Means, Louvain) on `pca_embeddings.parquet` or `umap_embeddings.parquet` to identify distinct cell populations or sub-populations within the cornea.
* **Classification:** Build models to classify cell types, or potentially age groups (if age metadata is present) using `pca_embeddings.parquet` or `umap_embeddings.parquet` as features. `cell_metadata.parquet` provides the necessary labels.
* **Dimensionality Reduction & Visualization:** Use the PCA and UMAP embeddings for generating 2D or 3D plots to visualize complex cell relationships and tissue organization.
* **Feature Selection:** Identify key genes or principal components relevant to specific corneal cell functions or age-related processes.

### Direct Download and Loading from Hugging Face Hub
This dataset is hosted on the Hugging Face Hub, allowing for easy programmatic download and loading of its component files.

```python
import pandas as pd
from huggingface_hub import hf_hub_download
import os

# Define the Hugging Face repository ID and the local directory for downloads
HF_REPO_ID = "longevity-db/human-cornea-snRNAseq"
LOCAL_DATA_DIR = "downloaded_human_cornea_data"

os.makedirs(LOCAL_DATA_DIR, exist_ok=True)
print(f"Created local download directory: {LOCAL_DATA_DIR}")

# List of Parquet files to download
parquet_files = [
    "expression.parquet",
    "gene_metadata.parquet",
    "cell_metadata.parquet",
    "pca_embeddings.parquet",
    "pca_explained_variance.parquet",
    "umap_embeddings.parquet",
    "highly_variable_gene_metadata.parquet",
    "gene_statistics.parquet"
]

# Download each file
downloaded_paths = {}
for file_name in parquet_files:
    try:
        path = hf_hub_download(repo_id=HF_REPO_ID, filename=file_name, local_dir=LOCAL_DATA_DIR)
        downloaded_paths[file_name] = path
        print(f"Downloaded {file_name} to: {path}")
    except Exception as e:
        print(f"Warning: Could not download {file_name}. It might not be in the repository or its name differs. Error: {e}")

# Load core Parquet files into Pandas DataFrames
df_expression = pd.read_parquet(downloaded_paths["expression.parquet"])
df_pca_embeddings = pd.read_parquet(downloaded_paths["pca_embeddings.parquet"])
df_umap_embeddings = pd.read_parquet(downloaded_paths["umap_embeddings.parquet"])
df_cell_metadata = pd.read_parquet(downloaded_paths["cell_metadata.parquet"])
df_gene_metadata = pd.read_parquet(downloaded_paths["gene_metadata.parquet"])
df_pca_explained_variance = pd.read_parquet(downloaded_paths["pca_explained_variance.parquet"])
df_hvg_metadata = pd.read_parquet(downloaded_paths["highly_variable_gene_metadata.parquet"])
df_gene_stats = pd.read_parquet(downloaded_paths["gene_statistics.parquet"])


print("\n--- Data Loaded from Hugging Face Hub ---")
print("Expression data shape:", df_expression.shape)
print("PCA embeddings shape:", df_pca_embeddings.shape)
print("UMAP embeddings shape:", df_umap_embeddings.shape)
print("Cell metadata shape:", df_cell_metadata.shape)
print("Gene metadata shape:", df_gene_metadata.shape)
print("PCA explained variance shape:", df_pca_explained_variance.shape)
print("HVG metadata shape:", df_hvg_metadata.shape)
print("Gene statistics shape:", df_gene_stats.shape)
```

-----

## Citation

Please ensure you cite the original source of the Human Cornea Atlas data from GSE186433.

**Original Publication:** Català, P., Groen, N., Dehnen, J. A., Soares, E., et al. (2021). "Single cell transcriptomics reveals the heterogeneity of the human cornea to identify novel markers of the limbus and stroma." *Scientific Reports*, 11(1), 21727.
**PMID:** 34741068
**DOI:** [10.1038/s41598-021-01188-7](https://www.google.com/search?q=https://doi.org/10.1038/s41598-021-01188-7)

**NCBI GEO Accession:** [GSE186433](https://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?acc=GSE186433)

If you use the `scanpy` library for any further analysis or preprocessing, please also cite Scanpy.

## Contributions

This dataset was processed and prepared by:

  - Venkatachalam
  - Pooja
  - Albert

*Curated on June 15, 2025.*

**Hugging Face Repository:** [https://huggingface.co/datasets/longevity-db/human-cornea-snRNAseq](https://www.google.com/search?q=https://huggingface.co/datasets/longevity-db/human-cornea-snRNAseq)