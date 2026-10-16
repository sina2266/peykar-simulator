# Universal Biological Simulation Platform
## Final Architecture Blueprint v3.0
### معماری جامع شبیه‌سازی، ترکیب و تکامل مدل‌های زیستی

**وضعیت: طرح معماری پیشنهادی | اکتبر ۲۰۲۶**

پس از بازبینی طراحی‌های قبلی، فایل Autonomous AI Research Laboratory، معماری پروژه‌های متن‌باز موجود و سناریوهای مختلف، به نظرم باید طراحی را یک مرحله اساسی جلوتر ببریم.

**هدف ما صرفاً ساخت یک Simulator نیست. هدف ساخت یک زیرساخت عمومی برای مدل‌سازی و شبیه‌سازی سیستم‌های زنده است که بتواند از مدل‌های موجود استفاده کند، مدل‌های جدید بسازد، آن‌ها را به هم متصل کند و با داده‌های تجربی بهبود دهد.**

اسم پیشنهادی را حفظ می‌کنم:

**UBSP: Universal Biological Simulation Platform**

اما قبل از ورود به معماری، سه تصمیم تعیین‌کننده داریم.

### تصمیم اول: ترکیب مدل‌ها مهم‌تر از تعداد آن‌هاست

اگر صد مدل زیستی داشته باشیم که نتوانند اطلاعات معنادار با هم تبادل کنند، فقط صد ابزار مستقل داریم.

اگر پنج مدل معتبر داشته باشیم که تعاملات آن‌ها درست تعریف شده باشد، می‌توانیم سؤالات علمی پیچیده‌تری را بررسی کنیم.

پس مزیت اصلی پلتفرم باید **Scientific Model Composability** باشد.

### تصمیم دوم: ساختار سلسله‌مراتبی لازم است، اما کافی نیست

یک سلول ممکن است در چند بافت وجود داشته باشد. یک پروتئین ممکن است در چند نوع سلول عملکرد متفاوتی داشته باشد. یک ماده شیمیایی ممکن است از کبد عبور کند و سپس روی چشم اثر بگذارد.

بنابراین به سه ساختار هم‌زمان نیاز داریم:

- **Hierarchy:** هر قسمت از چه اجزایی تشکیل شده است؟
- **Interaction Network:** اجزا چگونه روی یکدیگر اثر می‌گذارند؟
- **Dynamic Simulation:** این ارتباطات در طول زمان چه تغییری ایجاد می‌کنند؟

در کنار این‌ها، باید تاریخچه و اعتبار علمی مدل‌ها نیز مستقل نگهداری شود.

### تصمیم سوم: داده زیاد معادل دانش زیاد نیست

فرض کن یک میلیون تصویر از قرنیه داشته باشیم.

این تصاویر ممکن است برای شناخت ساختار و وضعیت بافت عالی باشند، ولی لزوماً به ما نمی‌گویند اگر یک ژن مشخص را تغییر دهیم، دو هفته بعد چه اتفاقی می‌افتد.

برای پاسخ به آن سؤال به داده مداخله‌ای، اندازه‌گیری‌های زمانی و مدل‌های مکانیزمی نیاز داریم.

این مشکل در پژوهش‌های Whole-Cell Modeling هم گزارش شده است. حتی ساخت مدل نسبتاً کامل از یک سلول، به‌دلیل کمبود داده‌های مناسب، ناهمخوانی داده‌ها و هزینه محاسباتی، هنوز یک چالش بزرگ علمی است. [PubMed Central (PMC)](https://pmc.ncbi.nlm.nih.gov/articles/PMC10661945/?utm_source=chatgpt.com)

بنابراین:

**سیستم باید بتواند آنچه را می‌داند شبیه‌سازی کند، آنچه را نمی‌داند مشخص کند و برای کاهش این نادانسته‌ها از داده و آزمایش استفاده کند.**

---

# بخش اول: تعریف و محدوده پروژه

## ۱. این پلتفرم دقیقاً چه محصولی است؟

UBSP یک **General-Purpose Multiscale Biological Modeling and Simulation Platform** خواهد بود.

سه نوع استفاده اصلی دارد:

| نوع استفاده | هدف | مثال |
|---|---|---|
| Biological Simulation | شبیه‌سازی رفتار سیستم زنده | رشد و ترمیم قرنیه |
| Virtual Experimentation | بررسی مداخلات فرضی | تغییر فعالیت ژن و مشاهده پاسخ سلولی |
| Model Development | ایجاد و بهبود مدل زیستی | بهبود vCornea با داده آزمایشگاهی |

این سه قابلیت باید مستقل باشند ولی بتوانند با یکدیگر کار کنند.

مثلاً ممکن است پژوهشگری فقط بخواهد مدل قلبی منتشرشده‌ای را اجرا کند.

شخص دیگری ممکن است بخواهد دو مدل عصبی را ترکیب کند.

پژوهشگر دیگری ممکن است دیتاست جدیدی داشته باشد و بخواهد از آن یک مدل پیش‌بینی‌کننده بسازد.

در هر سه حالت باید از یک زیرساخت مشترک استفاده شود.

## ۲. رابطه با Autonomous AI Research Laboratory

در فایل ارسالی، پروژه دیگر یک General-Purpose Scientific Research System است که سه بخش اصلی Scientific Method Core، Domain Packs و Instrument Adapters دارد. Pasted text

آن پروژه را تغییر نمی‌دهیم.

UBSP یک محصول مستقل است که می‌تواند از طریق Adapter در نقش یک Instrument قدرتمند به آن متصل شود.

```text
           AUTONOMOUS AI RESEARCH LAB
                  Separate Project
                         |
                         |
                 Instrument Adapter
                         |
                         v
+--------------------------------------------------+
|                                                  |
|          UNIVERSAL BIOLOGICAL SIMULATOR          |
|                                                  |
|   Biological Knowledge                           |
|   Existing Simulation Models                     |
|   Model Development                              |
|   Model Composition                              |
|   Multiscale Simulation                          |
|   Genetic & Other Interventions                  |
|   Data Integration                               |
|   Validation                                     |
|   Model Improvement                              |
|                                                  |
+--------------------------------------------------+
                         |
                         v
            Scientific Results + Evidence
```

مسئولیت‌ها باید شفاف باشند.

**Research Lab** سؤال علمی و هدف تحقیق را مدیریت می‌کند و بر مبنای شواهد درباره مسیر تحقیق تصمیم می‌گیرد.

**UBSP** مسئول مدل‌سازی، اجرای شبیه‌سازی، محاسبه نتایج، ارزیابی مدل‌ها و توسعه قابلیت‌های شبیه‌سازی است.

UBSP می‌تواند خودش ابزار طراحی و مقایسه آزمایش‌های محاسباتی داشته باشد. اما مدیریت برنامه‌های تحقیقاتی بزرگ و تصمیم‌گیری علمی عمومی همچنان متعلق به Research Lab است.

این تفکیک از ساخت دو سیستم تحقیقاتی تکراری جلوگیری می‌کند.

---

# بخش دوم: معماری جامع سیستم

## ۳. معماری سطح بالا

پس از بازبینی، پیشنهاد می‌کنم پلتفرم شامل **۱۰ زیرسیستم اصلی و چهار سرویس زیرساختی مشترک** باشد.

```text
                         USERS
                           |
            +--------------+--------------+
            |                             |
            v                             v
      VISUAL WORKSPACE              PUBLIC API / SDK
            |                             |
            +--------------+--------------+
                           |
                           v
+-------------------------------------------------------------+
|                                                             |
|                 BIOLOGICAL EXPLORER                         |
|                                                             |
|        Hierarchy | Graph | Search | Model Discovery         |
|                                                             |
+----------------------------+--------------------------------+
                             |
                             v
+-------------------------------------------------------------+
|                                                             |
|                   EXPERIMENT STUDIO                         |
|                                                             |
|  Scenario Definition | Intervention | Model Selection       |
|                                                             |
+----------------------------+--------------------------------+
                             |
             +---------------+---------------+
             |                               |
             v                               v
+--------------------------+    +-----------------------------+
| BIOLOGICAL KNOWLEDGE     |    | MODEL & CAPABILITY REGISTRY |
|                          |    |                             |
| Anatomy                  |    | Published Models            |
| Genes                    |    | Imported Simulators         |
| Proteins                 |    | Custom Models               |
| Cell Types               |    | ML Models                   |
| Interactions             |    | Composite Models            |
| Mechanisms               |    | Model Versions              |
| Ontologies               |    | Applicability               |
+-------------+------------+    +---------------+-------------+
              |                                 |
              +----------------+----------------+
                               |
                               v
+-------------------------------------------------------------+
|                                                             |
|                MODEL COMPOSITION ENGINE                     |
|                                                             |
|  Compatibility | Coupling | Dependencies | Scale Mapping    |
|                                                             |
+----------------------------+--------------------------------+
                             |
                             v
+-------------------------------------------------------------+
|                                                             |
|                 INTERVENTION ENGINE                         |
|                                                             |
| Genetic | Chemical | Cellular | Mechanical | Environment    |
|                                                             |
+----------------------------+--------------------------------+
                             |
                             v
+-------------------------------------------------------------+
|                                                             |
|               MULTISCALE SIMULATION RUNTIME                 |
|                                                             |
| Scheduler | State | Events | Feedback | Checkpoints          |
|                                                             |
+----------------------------+--------------------------------+
                             |
             +---------------+---------------+
             |                               |
             v                               v
+--------------------------+    +-----------------------------+
| SIMULATION ENGINES       |    | ML / SURROGATE ENGINES      |
|                          |    |                             |
| Molecular                |    | Perturbation Prediction     |
| Cellular                 |    | Learned Dynamics            |
| Tissue                   |    | Reduced-Order Models        |
| Neural                   |    | Statistical Models          |
| Organ                    |    | Hybrid Models               |
| Whole Body               |    |                             |
+-------------+------------+    +---------------+-------------+
              |                                 |
              +----------------+----------------+
                               |
                               v
+-------------------------------------------------------------+
|                                                             |
|                OBSERVATION & RESULTS                        |
|                                                             |
| Trajectories | Images | Fields | Metrics | Uncertainty       |
|                                                             |
+----------------------------+--------------------------------+
                             |
                             v
+-------------------------------------------------------------+
|                                                             |
|                 VALIDATION & MODEL LEARNING                 |
|                                                             |
| Calibration | Benchmarking | Refinement | Model Evolution    |
|                                                             |
+----------------------------+--------------------------------+
                             |
                             v
                     MODEL REGISTRY
                      NEW VERSIONS
```

چهار سرویس مشترک زیر تمام این بخش‌ها قرار می‌گیرند:

| Shared Service | مسئولیت |
|---|---|
| **Scientific Data Fabric** | دریافت، استانداردسازی، نگهداری و جست‌وجوی داده |
| **Scientific Provenance & Trust** | اعتبار، تاریخچه، شواهد و نسخه‌بندی |
| **Compute & Storage Infrastructure** | اجرای مدل‌ها، صف‌ها، GPU/HPC و ذخیره نتایج |
| **Scientific Governance** | مجوزها، ایمنی، کنترل تغییرات و ارزیابی مستقل |

این تقسیم‌بندی منطقی است، نه الزام به ساخت چهارده Microservice از روز اول.

در MVP همه این اجزا می‌توانند در یک **Modular Monolith** قرار بگیرند.

---

# بخش سوم: مدل داده و نمایش بیولوژی

## ۴. ساختار زیستی باید چندلایه باشد

پیشنهاد قبلی را حفظ می‌کنم، ولی یک اصلاح مهم به آن اضافه می‌کنم.

به‌جای یک درخت واحد، باید چند نمای مرتبط از یک دنیای زیستی داشته باشیم.

### ۴.۱. Structural Hierarchy

ساختار آناتومیکی و فیزیکی:

```text
Organism
└── Human
    ├── Nervous System
    ├── Circulatory System
    ├── Immune System
    └── Visual System
        ├── Eye
        │   ├── Cornea
        │   │   ├── Epithelium
        │   │   ├── Stroma
        │   │   └── Endothelium
        │   ├── Lens
        │   ├── Retina
        │   └── Optic Nerve
        └── Visual Pathways
```

این فقط نمای ساده‌ای از روابط است. در مدل علمی واقعی، بخش‌هایی مثل عصب بینایی و شبکیه می‌توانند هم‌زمان به چند گروه ساختاری یا عملکردی مربوط باشند.

### ۴.۲. Molecular Hierarchy

```text
Genome
├── Chromosomes
│   └── DNA Regions
│       ├── Genes
│       └── Regulatory Elements
│
├── Gene Expression
│   └── RNA
│
└── Molecular Processes
    ├── Protein Production
    ├── Protein Modification
    ├── Signaling
    └── Metabolism
```

این ساختار یک نمای کاربردی از ژنوم و فرایندهاست، نه اینکه تمام فرایندهای مولکولی واقعاً زیرمجموعه فیزیکی Genome باشند.

باید روابط ساختاری را از روابط فرایندی جدا کنیم.

### ۴.۳. Cellular Hierarchy

```text
Cell
├── Identity
│   ├── Cell Type
│   └── Lineage
│
├── Physical Structure
│   ├── Membrane
│   ├── Nucleus
│   └── Organelles
│
├── Internal State
│   ├── Gene Expression
│   ├── Protein Activity
│   ├── Metabolism
│   └── Cell Cycle
│
└── Behaviors
    ├── Growth
    ├── Division
    ├── Migration
    ├── Differentiation
    ├── Secretion
    └── Death
```

### ۴.۴. Dynamic Interaction Graph

ارتباط میان اجزا:

```text
Gene A
   |
   v
Protein A
   |
   v
Signaling Pathway
   |
   v
Cell Behavior
   |
   v
Tissue Behavior
   |
   v
Organ Function
```

این Graph باید مواردی مثل فعال‌سازی، مهار، حمل‌ونقل، اتصال فیزیکی، انتقال سیگنال و تغییر هویت سلولی را پشتیبانی کند.

همچنین روابط می‌توانند وابسته به زمان، مکان، دوز، گونه و وضعیت سلول باشند.

---

## ۵. یک اصلاح مهم: Entity، Model و Instance متفاوت‌اند

این مورد در طراحی قبلی به‌اندازه کافی پررنگ نبود.

سه مفهوم داریم:

**Biological Entity:** یک موجودیت زیستی واقعی یا تعریف‌شده، مثل EGF.

**Model:** یک نمایش محاسباتی از رفتار EGF یا اثر آن.

**Instance:** یک نمونه مشخص از مدل، با وضعیت و پارامترهای معین.

مثلاً:

```text
Biological Entity:
    EGF

Available Models:
    EGF Diffusion Model A
    EGF Receptor Binding Model B

Runtime Instances:
    EGF Field in Cornea
    EGF Field in Skin
```

وجود یک پروتئین مشترک در دو بافت، به معنی یکسان بودن رفتار آن نیست.

ممکن است غلظت، گیرنده‌ها و مسیرهای پیام‌رسانی متفاوت باشند.

**نتیجه معماری:** Knowledge Graph نباید مستقیماً محل نگهداری وضعیت محاسباتی تمام سلول‌های یک شبیه‌سازی باشد.

گراف، دانش و روابط را نگه می‌دارد.

Simulation State Store، وضعیت متغیر و حجیم مدل را نگه می‌دارد.

مثلاً موقعیت میلیون‌ها سلول در هزاران گام زمانی بهتر است در فایل‌ها و ساختارهای عددی تخصصی ذخیره شود، نه به‌صورت میلیون‌ها Node در Graph Database.

---

## ۶. Shared Biological Library

کتابخانه مشترک را از ابتدا طراحی می‌کنیم.

اما به‌جای یک کلاس عمومی عظیم به نام `Cell` که تمام رفتارها را در خود داشته باشد، از Component Composition استفاده می‌کنیم.

```text
SHARED BIOLOGICAL LIBRARY
│
├── Molecular Mechanisms
│   ├── Binding
│   ├── Diffusion
│   ├── Reaction Kinetics
│   └── Gene Regulation
│
├── Cellular Mechanisms
│   ├── Growth
│   ├── Cell Cycle
│   ├── Differentiation
│   ├── Migration
│   ├── Death
│   └── Metabolism
│
├── Tissue Mechanisms
│   ├── Adhesion
│   ├── Extracellular Matrix
│   ├── Mechanical Stress
│   ├── Healing
│   └── Remodeling
│
├── Physiological Mechanisms
│   ├── Blood Flow
│   ├── Drug Transport
│   ├── Neural Signaling
│   ├── Immune Signaling
│   └── Hormonal Regulation
│
└── Common Mathematical Components
    ├── ODE Models
    ├── PDE Models
    ├── Stochastic Processes
    ├── Agent Rules
    └── Statistical Models
```

این ساختار اجازه می‌دهد بخش‌های مشترک در مدل‌های مختلف استفاده شوند.

اما **اشتراک کد، با اشتراک اعتبار علمی متفاوت است.**

مثلاً یک معادله انتشار می‌تواند در دو مدل استفاده شود، ولی ضریب انتشار و شرایط مرزی در آن‌ها متفاوت است.

این تفاوت باید در Manifest مدل مشخص باشد.

---

# بخش چهارم: ورود شبیه‌سازهای موجود به پلتفرم

## ۷. استراتژی اصلی: Reuse First, Build When Necessary

یکی از تصمیم‌های مهم این است که قرار نیست همه شبیه‌سازهای زیستی را خودمان بسازیم.

برای هر قابلیت جدید باید ابتدا بررسی کنیم آیا مدل مناسبی از قبل وجود دارد.

من چهار مسیر تعریف می‌کنم:

| مسیر | وضعیت | اقدام |
|---|---|---|
| **Reuse** | مدل موجود مناسب است | وارد کردن و استفاده از مدل |
| **Adapt** | مدل موجود بخشی از نیاز را پوشش می‌دهد | ساخت Adapter یا اصلاح کنترل‌شده |
| **Compose** | چند مدل هرکدام بخشی از مسئله را حل می‌کنند | اتصال مدل‌ها با Coupling معتبر |
| **Build** | مدل قابل‌استفاده وجود ندارد | ساخت مدل جدید از دانش و داده |

این چهار مسیر می‌توانند ترکیب شوند.

مثلاً ممکن است یک مدل از مقاله علمی وارد کنیم، یک مدل دیگر را از داده‌های تجربی بسازیم و بعد هر دو را با یک مدل واسط به هم متصل کنیم.

## ۷.۱. چه پروژه‌های موجودی برای ما ارزشمندند؟

بررسی منابع موجود نشان می‌دهد بخش بزرگی از زیرساخت موردنیاز ما قبلاً در ابزارهای تخصصی توسعه یافته است.

| پروژه | تخصص | نقش احتمالی در UBSP |
|---|---|---|
| [CompuCell3D](https://github.com/CompuCell3D/CompuCell3D) | مدل‌سازی چندسلولی و بافت | موتور شبیه‌سازی سلولی |
| [vCornea](https://github.com/VaninJoel/vCornea) | اپی‌تلیوم قرنیه | اولین Model Package واقعی |
| [PhysiCell](https://physicell.org/) | رفتار سلول‌ها در محیط‌های سه‌بعدی | مدل‌های چندسلولی جایگزین |
| [VCell](https://vcell.org/) | واکنش‌ها و انتشار مولکولی | مدل‌های درون‌سلولی |
| [COPASI](https://copasi.org/) | شبکه‌های واکنش بیوشیمیایی | مدل‌های مولکولی و کالیبراسیون |
| [Chaste](https://chaste.github.io/) | مدل‌سازی چندمقیاسی بافت و فیزیولوژی | مدل‌های بافت، قلب و الکتروفیزیولوژی |
| [NEURON](https://www.neuronsimulator.org/) | نورون‌ها و شبکه‌های عصبی | شبیه‌سازی فعالیت عصبی |
| [PK-Sim](https://github.com/Open-Systems-Pharmacology/PK-Sim) | توزیع و متابولیسم مواد در بدن | مدل‌های چنداندامی |
| [MoBi](https://github.com/Open-Systems-Pharmacology/MoBi) | مدل‌های مولکولی و فیزیولوژیکی چندمقیاسی | ترکیب مدل‌های فیزیولوژی |
| [GEARS](https://github.com/snap-stanford/GEARS) | پیش‌بینی پاسخ بیان ژن به مداخله | مدل ML ژنتیکی |

چند نکته از بررسی‌ها:

**CompuCell3D** یک محیط تخصصی چندمقیاسی و چندسلولی با هسته C++ و رابط Python است. بنابراین برای مدل‌هایی مثل vCornea مناسب است. [GitHub](https://github.com/CompuCell3D/CompuCell3D?utm_source=chatgpt.com)

**VCell** از مدل‌های واکنش، انتشار، شبیه‌سازی تصادفی و مدل‌های ترکیبی پشتیبانی می‌کند. نسخه ۸ آن همچنین امکانات بیشتری برای مدل‌سازی فضایی مولکولی فراهم کرده است. [vcell.org](https://vcell.org/?utm_source=chatgpt.com)

**Chaste** به‌طور مشخص برای مسائل چندمقیاسی زیستی، از جمله الکتروفیزیولوژی قلب و جمعیت‌های سلولی، توسعه یافته است. [Chaste](https://chaste.github.io/?utm_source=chatgpt.com)

**PK-Sim و MoBi** از نظر معماری برای ما بسیار جالب هستند. آن‌ها از Building Blockهای قابل‌استفاده مجدد بهره می‌برند و می‌توانند مدل‌های مرتبط با مولکول‌ها و کل بدن را ترکیب کنند. این نمونه عملی خوبی از ارزش Composition در بیولوژی است. [GitHub](https://github.com/Open-Systems-Pharmacology/Suite/blob/develop/README.md?utm_source=chatgpt.com)

**GEARS** می‌تواند پاسخ رونویسی سلولی به برخی مداخلات ژنی را پیش‌بینی کند؛ اما طبق توضیح رسمی توسعه‌دهندگان، مدل اصلی برای انتقال مستقیم پیش‌بینی بین انواع مختلف سلول طراحی نشده است. [GitHub](https://github.com/snap-stanford/GEARS?utm_source=chatgpt.com)

نتیجه: نباید برای تمام ابزارها یک سطح اعتبار یا یک قرارداد علمی ثابت فرض کنیم.

---

## ۸. Model Discovery & Import Pipeline

هر مدلی که از بیرون وارد سیستم می‌شود باید یک فرایند مشخص را طی کند.

```text
               NEW SCIENTIFIC MODEL
                        |
                        v
                 Source Discovery
                        |
                        v
                 Source Inspection
                        |
                        v
              License & Access Check
                        |
                        v
               Capability Extraction
                        |
                        v
              Dependency Inspection
                        |
                        v
              Isolated Execution Test
                        |
                        v
              Original Result Replay
                        |
                        v
              Scientific Model Mapping
                        |
                        v
                 Adapter Generation
                        |
                        v
              Compatibility Assessment
                        |
                        v
                 Model Registration
                        |
                        v
               Independent Validation
```

### این مراحل دقیقاً چه می‌کنند؟

**Source Discovery:** مدل را از GitHub، BioModels، ModelDB، مقالات علمی یا مخازن دیگر پیدا می‌کنیم.

**Source Inspection:** کد، معادلات، مستندات و فایل‌های اصلی بررسی می‌شوند.

**License Check:** مشخص می‌شود مجاز به اجرا، تغییر، توزیع یا استفاده تجاری از مدل هستیم یا نه.

**Capability Extraction:** سیستم تشخیص می‌دهد مدل واقعاً چه پدیده‌ای را شبیه‌سازی می‌کند.

**Execution Test:** مدل در محیط ایزوله اجرا می‌شود.

**Original Result Replay:** بررسی می‌کنیم آیا می‌توانیم بخشی از نتایج منتشرشده را بازتولید کنیم.

**Scientific Mapping:** متغیرهای اختصاصی مدل به موجودیت‌ها و کمیت‌های استاندارد نگاشت می‌شوند.

**Adapter:** یک رابط استاندارد ساخته می‌شود.

**Independent Validation:** مدل برای کاربرد مشخصی با شواهد مستقل ارزیابی می‌شود.

### یک اصلاح مهم

**اجرای موفق یک مدل به معنی اعتبار علمی آن نیست.**

ممکن است تمام تست‌های فنی پاس شوند، اما مدل در پیش‌بینی شرایط جدید ضعیف عمل کند.

بنابراین دو Status مستقل داریم:

`EXECUTION_READY`

و

`SCIENTIFICALLY_VALIDATED_FOR_CONTEXT`

دومی فقط پس از ارزیابی مناسب به دست می‌آید.

---

## ۹. چهار شکل متفاوت وارد کردن مدل

همه مدل‌های علمی در قالب یک Repository مرتب ارائه نمی‌شوند.

بنابراین Importer باید چند مسیر داشته باشد.

### حالت A: مدل استاندارد و آماده

مثلاً یک مدل SBML داریم.

می‌توانیم آن را با COPASI اجرا کنیم و نتایج را دریافت کنیم.

این ساده‌ترین حالت است.

### حالت B: یک GitHub Repository مستقل

مثل vCornea.

کد و فرمت اختصاصی دارد.

باید محیط اجرای اصلی آن را حفظ کنیم، Adapter بنویسیم و خروجی‌ها را استاندارد کنیم.

### حالت C: مدل فقط داخل مقاله توضیح داده شده

ممکن است مقاله شامل معادلات، پارامترها و نمودارها باشد، ولی کد منتشر نشده باشد.

در این حالت باید مدل را دوباره پیاده‌سازی کنیم.

این بازسازی باید دارای وضعیت:

`RECONSTRUCTED_FROM_PUBLICATION`

باشد.

تا زمانی که با نتایج اصلی مقایسه نشده، نباید آن را معادل مدل نویسندگان بدانیم.

### حالت D: مدل آماده‌ای نداریم، فقط داده داریم

مثلاً یک مجموعه Time-lapse از رشد سلول‌ها داریم.

می‌توانیم مدل جدیدی از رفتار سلول بسازیم.

اما نوع مدل باید با اطلاعات موجود سازگار باشد.

اگر فقط تصاویر نهایی داشته باشیم، ممکن است نتوانیم دینامیک واقعی رشد سلول‌ها را شناسایی کنیم.

این مسیر را **Data-Driven Model Construction** می‌نامیم.

---

# بخش پنجم: Model Registry و Model Contracts

## ۱۰. هر مدل باید یک Scientific Model Package باشد

یکی از ضعف‌های طراحی‌های معمول این است که Model Registry فقط نام مدل و آدرس فایل را نگه می‌دارد.

برای ما این کافی نیست.

هر Model Package باید شامل این اطلاعات باشد:

| دسته | اطلاعات |
|---|---|
| Identity | شناسه، نام، نسخه و نویسنده |
| Biological Scope | گونه، سلول، بافت، اندام و وضعیت بیماری |
| Model Type | مکانیزمی، آماری، ML، ترکیبی |
| Scale | مقیاس زمانی، مکانی و زیستی |
| State | متغیرهای وضعیت داخلی |
| Inputs | ورودی‌ها با نوع و واحد |
| Outputs | خروجی‌ها با نوع و واحد |
| Interventions | تغییرات پشتیبانی‌شده |
| Dependencies | مدل‌ها و کتابخانه‌های وابسته |
| Engine | موتور اجرایی موردنیاز |
| Assumptions | فرضیات علمی |
| Constraints | محدودیت‌های فیزیکی و بیولوژیکی |
| Validity | شرایطی که مدل در آن‌ها بررسی شده |
| Evidence | منابع و داده‌های علمی |
| License | مجوز استفاده |
| Provenance | تاریخچه منشأ و تغییرات |

### ۱۰.۱. قرارداد پنج‌گانه هر مدل

پیشنهاد قبلی را دقیق‌تر می‌کنم.

هر Model Package باید پنج Interface اصلی داشته باشد:

**۱. Input Interface**

چه اطلاعاتی را دریافت می‌کند؟

**۲. Output Interface**

چه اطلاعاتی را تولید می‌کند؟

**۳. State Interface**

چه متغیرهای داخلی دارد و چگونه وضعیت خود را ذخیره و بازیابی می‌کند؟

**۴. Intervention Interface**

کدام پارامترها یا ویژگی‌های مدل قابل تغییرند؟

**۵. Observation Interface**

چگونه خروجی محاسباتی مدل به یک اندازه‌گیری قابل مقایسه با آزمایش واقعی تبدیل می‌شود؟

این مورد پنجم بسیار مهم است.

مثلاً در شبیه‌ساز، تعداد سلول‌های زنده را دقیقاً می‌دانیم.

اما در آزمایشگاه، ممکن است فقط تصویر میکروسکوپی و یک الگوریتم شمارش سلول داشته باشیم.

برای مقایسه منصفانه، باید تفاوت روش مشاهده را در نظر بگیریم.

---

## ۱۱. نمونه Model Manifest

مثال پیشنهادی برای مدل vCornea:

```yaml
model:
  id: human.eye.cornea.epithelium
  version: 0.1.0

  identity:
    name: V-Cornea Adapter
    source: VaninJoel/vCornea
    source_version: pinned_commit

  biology:
    species: Homo sapiens
    organ: eye
    tissue: corneal_epithelium

  representation:
    type: agent_based
    spatial_dimension: 2
    primary_scale: cellular_tissue

  execution:
    engine: compucell3d
    mode: isolated_process
    stochastic: true

  capabilities:
    - tissue_homeostasis
    - cell_proliferation
    - cell_differentiation
    - cell_migration
    - tissue_injury

  interventions:
    - chemical_exposure
    - cell_behavior_parameters

  outputs:
    - cell_population
    - tissue_thickness
    - recovery_metrics

  scientific_validity:
    status: independent_validation_required

  provenance:
    source_paper_required: true
    source_commit_required: true
    environment_digest_required: true

  limitations:
    - no_full_eye_simulation
    - simplified_basement_membrane
    - no_explicit_genome_dynamics
```

این یک **Manifest نمونه برای Adapter آینده** است، نه فایل رسمی پروژه.

مثلاً ورودی ژنتیکی را به این مدل اضافه نکرده‌ام، چون vCornea فعلی مستقیماً شبیه‌ساز ژنوم نیست.

اگر روزی مدل ژنتیکی به آن وصل شود، Capability جدید باید به نسخه جدید اضافه شود.

## ۱۲. Model Registry باید Capability-Based باشد

فرض کن پژوهشگر می‌خواهد فعالیت ژنی را شبیه‌سازی کند.

نباید لازم باشد بداند GEARS چیست.

سیستم باید بتواند بر اساس Capability جست‌وجو کند.

مثلاً:

```text
Required Capability:
  predict_transcriptional_response

Context:
  species: human
  cell_type: retinal_ganglion_cell

Intervention:
  gene_activity_change
```

Registry باید مدل‌های کاندیدا را پیدا کند.

سپس بررسی کند:

- آیا مدل برای سلول انتخاب‌شده آموزش دیده؟
- آیا داده آموزشی مناسب وجود دارد؟
- آیا نوع مداخله پشتیبانی می‌شود؟
- آیا مدل برای ترکیب چند مداخله معتبر است؟
- آیا خروجی برای مسئله فعلی مناسب است؟

اگر هیچ مدل معتبری وجود نداشت، نتیجه باید:

`NO_VALID_MODEL_AVAILABLE`

باشد.

نه اینکه سیستم نزدیک‌ترین مدل را انتخاب کند و بدون هشدار اجرا کند.

---

# بخش ششم: Model Composition و Interaction Engine

## ۱۳. اتصال دو مدل فقط وصل کردن Input و Output نیست

فرض کن دو مدل داریم:

**Model A:** غلظت EGF را پیش‌بینی می‌کند.

**Model B:** رشد سلول‌ها را بر اساس EGF پیش‌بینی می‌کند.

اگر Model A غلظت را در واحد `ng/mL` تولید کند، اما Model B انتظار واحد دیگری داشته باشد، اتصال مستقیم می‌تواند اشتباه باشد.

حتی اگر واحدها یکسان باشند، ممکن است:

- یکی غلظت بیرون سلول را گزارش کند.
- دیگری غلظت داخل سلول را بخواهد.
- یکی میانگین یک ساعت را تولید کند.
- دیگری مقدار لحظه‌ای را نیاز داشته باشد.

بنابراین یک لایه واسط لازم داریم.

### Coupling Contract

هر ارتباط باید حداقل این ویژگی‌ها را داشته باشد:

```yaml
coupling:
  source:
    model: egf_diffusion
    output: extracellular_egf

  target:
    model: epithelial_cell_growth
    input: egf_concentration

  mapping:
    type: spatial_concentration_mapping

  synchronization:
    mode: time_averaged
    interval_seconds: 600

  scientific_validation:
    required: true
```

این مثال فرضی است.

`time_averaged` فقط وقتی قابل قبول است که میانگین‌گیری از نظر فیزیکی و بیولوژیکی مناسب باشد.

## ۱۴. انواع Interaction که باید پشتیبانی شوند

| نوع ارتباط | مثال |
|---|---|
| Chemical Signaling | EGF روی رشد سلول اثر می‌گذارد |
| Diffusion | اکسیژن در بافت منتشر می‌شود |
| Transport | دارو از خون به چشم می‌رسد |
| Mechanical | فشار داخل چشم بر عصب بینایی نیرو وارد می‌کند |
| Electrical | نورون سیگنال الکتریکی منتقل می‌کند |
| Cell-Cell Contact | تماس دو سلول بر رفتار آن‌ها اثر می‌گذارد |
| Metabolic | یک سلول ماده‌ای را مصرف می‌کند |
| Immune | سلول ایمنی به سیگنال آسیب پاسخ می‌دهد |
| Hormonal | پیام هورمونی روی بافت اثر می‌گذارد |
| Genetic Regulation | پروتئین تنظیم‌کننده بیان ژن را تغییر می‌دهد |
| Developmental | سیگنال رشد مسیر تمایز سلول را تغییر می‌دهد |
| Structural | یک بافت، پشتیبانی مکانیکی بافت دیگر را فراهم می‌کند |

هر Interaction باید ویژگی‌های زیر را ثبت کند:

- Source
- Target
- Mechanism
- Direction
- Biological Context
- Mathematical Representation
- Time Delay
- Spatial Mapping
- Evidence
- Uncertainty

برخی ارتباطات به بیش از دو جزء وابسته‌اند.

مثلاً اتصال یک لیگاند به یک گیرنده ممکن است در حضور یک مولکول کمکی تغییر کند.

بنابراین Graph باید از ارتباطات چندجزئی نیز پشتیبانی کند.

می‌توان آن‌ها را به‌صورت Reaction یا Process Node نمایش داد.

---

## ۱۵. Multi-Scale Coupling

مدل‌های مختلف ممکن است مقیاس‌های کاملاً متفاوت داشته باشند.

| سطح | نمونه مدل |
|---|---|
| Molecular | اتصال پروتئین به گیرنده |
| Intracellular | مسیر پیام‌رسانی داخل سلول |
| Cellular | رشد و تقسیم سلول |
| Tissue | ترمیم قرنیه |
| Organ | عملکرد نوری چشم |
| Organ System | پردازش اطلاعات بینایی |
| Organism | پاسخ عمومی بدن |

پیشنهاد می‌کنم Model Composer از سه الگوی اصلی پشتیبانی کند.

### A. Sequential Coupling

خروجی یک مدل به مدل بعدی داده می‌شود.

مثلاً:

```text
Molecular Model
      |
      v
Cellular Model
      |
      v
Tissue Model
```

برای فرایندهایی که وابستگی یک‌طرفه دارند مناسب است.

### B. Bidirectional Coupling

دو مدل دائماً روی هم اثر می‌گذارند.

مثلاً:

```text
Oxygen Diffusion
       |
       v
Cell Metabolism
       |
       v
Oxygen Consumption
       |
       +--------> Oxygen Diffusion
```

Runtime باید این بازخورد را در طول زمان مدیریت کند.

### C. Hierarchical Coupling

یک مدل بزرگ‌تر از نتایج مدل‌های جزئی استفاده می‌کند.

مثلاً مدل کل چشم می‌تواند از مدل‌های قرنیه، عدسی و شبکیه استفاده کند.

اما در بعضی آزمایش‌ها لازم نیست تمام جزئیات سلولی را محاسبه کنیم.

در این حالت می‌توان از مدل ساده‌شده استفاده کرد.

**نکته:** انتخاب مدل کم‌جزئیات باید با شواهد معتبر شود. کمتر محاسبه کردن نباید به معنی حذف خودسرانه مکانیزم‌های مهم باشد.

---

## ۱۶. Numerical Coupling و پایداری محاسباتی

این یکی از مهم‌ترین چیزهایی است که به طراحی قبلی اضافه می‌کنم.

حتی وقتی ارتباط بیولوژیکی دو مدل درست است، اتصال عددی ممکن است اشتباه باشد.

مثلاً:

- هر مدل به‌تنهایی پایدار است.
- اما وقتی به هم وصل می‌شوند، غلظت یک ماده غیرواقعی افزایش پیدا می‌کند.
- یا جرم ماده در مرز دو مدل از بین می‌رود.
- یا یک Feedback Loop باعث نوسان مصنوعی می‌شود.

بنابراین Composition Engine به یک **Numerical Coupling Validator** نیاز دارد.

باید بتواند بررسی کند:

| تست | هدف |
|---|---|
| Unit Consistency | سازگاری ابعادی |
| Conservation | حفظ جرم، انرژی یا بار الکتریکی در شرایط مربوط |
| Time Synchronization | جلوگیری از خطای زمانی |
| Spatial Consistency | تطبیق هندسه و مختصات |
| Numerical Stability | جلوگیری از واگرایی |
| Boundary Conditions | سازگاری شرایط مرزی |
| Feedback Convergence | همگرایی ارتباطات دوطرفه |
| Solver Compatibility | سازگاری روش‌های عددی |

اگر دو مدل دارای ارتباط دوطرفه باشند، گاهی باید در هر گام زمانی چند بار حل شوند تا خروجی‌هایشان با هم سازگار شود.

در مهندسی سیستم‌ها استاندارد [FMI](https://fmi-standard.org/) برای تبادل مدل و Co-Simulation وجود دارد. از مفاهیم آن استفاده می‌کنیم، ولی برای موتورهای زیستی که پشتیبانی مستقیم ندارند Adapter تخصصی لازم خواهد بود. [fmi-standard.org](https://fmi-standard.org/?utm_source=chatgpt.com)

---

# بخش هفتم: Scientific Data Fabric

## ۱۷. انواع داده‌هایی که باید پشتیبانی شوند

داده‌های ما فقط CSV یا تصاویر میکروسکوپی نیستند.

در آینده ممکن است اطلاعاتی با اندازه‌های بسیار متفاوت، ساختارهای پیچیده و منشأهای مختلف داشته باشیم.

من Data Fabric را به هشت گروه تقسیم می‌کنم.

| گروه | نمونه داده | هدف |
|---|---|---|
| Molecular | DNA، RNA، پروتئین‌ها، متابولیت‌ها | شناخت مکانیزم‌های مولکولی |
| Single-Cell | اطلاعات سلول‌های منفرد | شناخت انواع و وضعیت سلول‌ها |
| Spatial | داده همراه با موقعیت مکانی | شناخت ساختار و تعاملات |
| Temporal | اندازه‌گیری‌های متوالی | یادگیری رفتار دینامیکی |
| Imaging | میکروسکوپی، OCT، MRI و غیره | اندازه‌گیری ساختار |
| Perturbational | داده قبل و بعد از مداخله | بررسی پاسخ به تغییر |
| Physiological | فشار، جریان خون، سیگنال عصبی | مدل فیزیولوژی |
| Experimental & Clinical | آزمایش‌های واقعی و داده بیماران | اعتبارسنجی و بررسی تعمیم |

بعضی داده‌ها می‌توانند در چند گروه قرار بگیرند.

مثلاً یک مجموعه Spatial Transcriptomics هم مولکولی است و هم مکانی.

### نکته مهم درباره داده‌های جدید

هر دیتاست باید دارای یک **Dataset Contract** باشد.

این قرارداد مشخص می‌کند:

```yaml
dataset:
  id: dataset_001

  source:
    repository: public_repository
    accession: example_id

  biological_context:
    species: human
    tissue: corneal_epithelium

  modality:
    type: time_lapse_microscopy

  observations:
    - wound_area
    - cell_movement

  experimental_design:
    temporal: true
    intervention: documented

  quality:
    status: pending_review

  intended_use:
    - model_calibration
    - model_validation

  provenance:
    required: true
```

در نسخه واقعی باید موارد دیگری مانند روش نمونه‌برداری، تجهیزات، Batch، تعداد اهداکنندگان و مجوز استفاده هم ثبت شوند.

---

## ۱۸. Data Sources و استانداردهای مناسب

برای اینکه پلتفرم محدود به یک ارائه‌دهنده نشود، Data Connectorها را مستقل طراحی می‌کنیم.

| منبع | چه چیزی می‌توانیم دریافت کنیم؟ |
|---|---|
| [NCBI GEO](https://www.ncbi.nlm.nih.gov/geo/) | داده‌های بیان ژن و مطالعات مولکولی |
| [CELLxGENE](https://cellxgene.cziscience.com/) | مجموعه‌های Single-Cell |
| [HuBMAP](https://portal.hubmapconsortium.org/) | داده‌های چندوجهی و مکانی از بافت‌های انسان |
| [Human Cell Atlas](https://www.humancellatlas.org/) | اطلس‌های سلولی |
| [scPerturb](https://www.sanderlab.org/scPerturb/) | داده‌های پاسخ سلولی به مداخله |
| [BioModels](https://www.ebi.ac.uk/biomodels/) | مدل‌های محاسباتی منتشرشده |
| [Physiome Model Repository](https://models.physiomeproject.org/) | مدل‌های فیزیولوژیکی و ریاضی |
| [ModelDB](https://modeldb.science/) | مدل‌های محاسباتی سیستم عصبی |
| [Hugging Face](https://huggingface.co/datasets) | دیتاست‌ها و مدل‌های منتشرشده |
| Local Laboratory | داده تولیدشده در تحقیقات خودمان |

مهم‌ترین نکته این است که **Model Repositories را از Experimental Data Repositories جدا کنیم**.

مثلاً BioModels عمدتاً محل دریافت مدل است.

اما scPerturb مجموعه‌ای از داده‌های تجربی مربوط به پاسخ سلول‌ها به مداخلات است. نسخه منتشرشده scPerturb شامل ۴۴ مجموعه داده هماهنگ‌سازی‌شده است. [nature.com](https://www.nature.com/articles/s41592-023-02144-y?utm_source=chatgpt.com)

هرکدام وارد Pipeline متفاوتی می‌شوند.

### فرمت‌های داده

پیشنهاد می‌کنم از استانداردهای موجود استفاده کنیم:

| نوع داده | فرمت پیشنهادی |
|---|---|
| Single-cell Omics | AnnData / H5AD |
| Spatial Omics | SpatialData |
| Microscopy | OME-Zarr / OME-TIFF |
| Time-Series | HDF5، Parquet، Zarr |
| مدل‌های بیوشیمیایی | SBML |
| مدل‌های فیزیولوژیکی | CellML |
| تعریف آزمایش شبیه‌سازی | SED-ML |
| بسته‌بندی مدل و آزمایش | COMBINE/OMEX |

[SpatialData](https://spatialdata.scverse.org/) برای مدیریت داده‌های Spatial Omics طراحی شده و از فرمت‌هایی مانند Zarr و Parquet استفاده می‌کند.

[OME-NGFF](https://ngff.openmicroscopy.org/) نیز یک استاندارد مهم برای نگهداری تصاویر زیستی حجیم و چندمقیاسی است. [spatialdata.scverse.org](https://spatialdata.scverse.org/en/latest/design_doc.html?utm_source=chatgpt.com)

نباید همه داده‌ها را مجبور کنیم به یک فرمت اختصاصی تبدیل شوند.

بهتر است Metadata و Interface مشترک داشته باشند، ولی فایل‌های اصلی در فرمت مناسب خودشان باقی بمانند.

---

## ۱۹. Data Ingestion Pipeline

این Pipeline باید بتواند ورود داده جدید را تا حد زیادی خودکار کند.

```text
                    NEW DATA
                       |
                       v
                Source Connector
                       |
                       v
              Metadata Extraction
                       |
                       v
              License / Privacy Check
                       |
                       v
                Format Inspection
                       |
                       v
                Quality Control
                       |
                       v
             Biological Annotation
                       |
                       v
              Ontology Normalization
                       |
                       v
             Cross-Dataset Matching
                       |
                       v
                 Duplicate Check
                       |
                       v
               Dataset Versioning
                       |
                       v
              Dataset Registry
                       |
           +-----------+-----------+
           |           |           |
           v           v           v
       Training    Calibration   Validation
```

### Cross-Dataset Matching

فرض کن دو آزمایشگاه اطلاعات یک نوع سلول را منتشر کرده‌اند.

یکی از نام `Basal Epithelial Cell` استفاده می‌کند.

دیگری نام متفاوتی برای جمعیت سلولی مشابه دارد.

سیستم باید با استفاده از شناسه‌های استاندارد تشخیص دهد که آیا واقعاً درباره یک نوع سلول صحبت می‌کنند یا نه.

اما یکسان‌سازی نام نباید تفاوت‌های واقعی زیستی را از بین ببرد.

### Dataset Lineage

اگر دیتاستی از سه مجموعه دیگر ساخته شده باشد، باید منشأ تمام رکوردهای آن مشخص بماند.

```text
Dataset A ──┐
            |
Dataset B ──┼──> Harmonized Dataset
            |           |
Dataset C ──┘           v
                    Model Training
                           |
                           v
                      Model v2
```

بعداً اگر مشخص شد Dataset B مشکل دارد، سیستم باید تمام مدل‌هایی را که از آن استفاده کرده‌اند پیدا کند.

این قابلیت را **Dataset-to-Model Dependency Tracking** می‌نامیم.

---

# بخش هشتم: ساخت مدل جدید از داده

## ۲۰. همه دیتاست‌ها نمی‌توانند یک نوع شبیه‌ساز تولید کنند

این مورد باید در معماری صریح باشد.

فرض کن سه دیتاست داریم:

**Dataset A:** هزاران تصویر ثابت از سلول‌ها.

**Dataset B:** تصاویر متوالی سلول‌ها در طول ۴۸ ساعت.

**Dataset C:** پاسخ سلول‌ها به چند مداخله ژنتیکی مشخص.

از هرکدام چیز متفاوتی می‌توان یاد گرفت.

| داده | چیزی که می‌توانیم مدل کنیم | چیزی که الزاماً نمی‌فهمیم |
|---|---|---|
| تصاویر ثابت | شکل و وضعیت ظاهری سلول | رفتار زمانی و علت تغییر |
| تصاویر متوالی | حرکت، رشد و تغییر شکل | تمام مکانیزم‌های مولکولی |
| داده Perturbation | پاسخ مشاهده‌شده به تغییرات | پاسخ به تمام تغییرات ناشناخته |

بنابراین یک **Model Feasibility Analyzer** لازم داریم.

قبل از ساخت مدل باید بررسی کند:

- سؤال علمی چیست؟
- کدام متغیرها قابل مشاهده‌اند؟
- چه داده‌ای موجود است؟
- چه مکانیزم‌هایی شناخته شده‌اند؟
- کدام متغیرها از داده قابل شناسایی هستند؟
- چه فرضیات اضافی لازم است؟
- چه نوع مدلی قابل دفاع است؟

این قسمت از تولید مدل‌های ظاهراً دقیق ولی بی‌پشتوانه جلوگیری می‌کند.

---

## ۲۱. Model Construction Pipeline

وقتی مدل آماده وجود ندارد، از این مسیر استفاده می‌کنیم:

```text
                RESEARCH QUESTION
                       |
                       v
              Available Data Search
                       |
                       v
              Mechanism Identification
                       |
                       v
              Model Feasibility Study
                       |
                       v
              Modeling Approach Selection
                       |
         +-------------+-------------+
         |             |             |
         v             v             v
     Mechanistic      ML Model     Hybrid Model
         |             |             |
         +-------------+-------------+
                       |
                       v
                Model Construction
                       |
                       v
               Parameter Estimation
                       |
                       v
                 Verification
                       |
                       v
              Independent Validation
                       |
                       v
                 Model Registry
```

### Modeling Approach Selection

سیستم باید بتواند میان روش‌های مختلف انتخاب کند.

| روش | مناسب برای |
|---|---|
| Rule-Based | رفتارهای ساده با قوانین مشخص |
| ODE | رفتار متغیرها در زمان |
| PDE | رفتارهای زمانی و مکانی |
| Agent-Based | تعامل سلول‌ها و موجودیت‌های جداگانه |
| Stochastic | فرایندهای تصادفی |
| Constraint-Based | مدل‌های مبتنی بر قیود، مثل بعضی شبکه‌های متابولیکی |
| Statistical | روابط آماری و عدم‌قطعیت |
| Machine Learning | روابط پیچیده قابل یادگیری از داده |
| Hybrid | ترکیب مکانیزم شناخته‌شده و مدل یادگیری‌محور |

مدل‌های مختلف می‌توانند داخل یک سیستم ترکیب شوند.

برای مثال بخشی از مدل، انتشار اکسیژن را با PDE محاسبه می‌کند و بخش دیگر، رفتار هر سلول را با Agent-Based Simulation اجرا می‌کند.

---

# بخش نهم: Genetic & Cellular Intervention Framework

## ۲۲. Intervention نباید فقط تغییر یک پارامتر باشد

در نسخه قبلی، مداخلات ژنتیکی و سلولی را اضافه کردیم.

من این بخش را گسترده‌تر می‌کنم.

**Intervention یک تغییر تعریف‌شده روی وضعیت، ساختار یا رفتار سیستم شبیه‌سازی‌شده است.**

مداخله می‌تواند محلی، سیستمی، موقت یا دائمی باشد.

### انواع Intervention

| دسته | مثال |
|---|---|
| Genetic | تغییر فرضی فعالیت یا عملکرد ژن |
| Epigenetic | تغییر وضعیت تنظیم بیان ژن |
| Molecular | تغییر غلظت یا فعالیت یک مولکول |
| Cellular | افزودن، حذف یا تغییر وضعیت سلول |
| Tissue | آسیب یا جایگزینی یک قسمت بافت |
| Mechanical | تغییر فشار، کشش یا نیرو |
| Chemical | تغییر مواد محیط |
| Electrical | تحریک الکتریکی |
| Environmental | تغییر دما، اکسیژن یا نور |
| Systemic | تغییر یک وضعیت فیزیولوژیکی در کل بدن |

این‌ها دسته‌های اجرایی Simulator هستند، نه ادعای امکان‌پذیری تمام این مداخلات در واقعیت.

## ۲۳. Genetic Intervention Pipeline

برای ژنتیک باید چند لایه جدا داشته باشیم.

```text
                 GENETIC INTERVENTION
                         |
                         v
             DNA / Gene-Level Change
                         |
                         v
                Regulatory Response
                         |
                         v
                 RNA / Protein State
                         |
                         v
               Intracellular Signaling
                         |
                         v
                   Cell Phenotype
                         |
                         v
                  Tissue Response
                         |
                         v
                  Organ Function
                         |
                         v
                Organism-Level Effects
```

اما تمام این مسیرها همیشه لازم نیستند.

گاهی یک تغییر ژنتیکی فقط در یک مدل سلولی بررسی می‌شود.

گاهی داده کافی برای اتصال به مدل بافت وجود ندارد.

گاهی هم هدف فقط پیش‌بینی RNA Expression است.

### دو نوع آزمایش ژنتیکی

**Direct State Perturbation**

مثلاً فرض می‌کنیم فعالیت یک ژن ۲۰٪ افزایش یافته است.

سؤال: این تغییر چه اثری بر مدل سلول دارد؟

**Intervention Mechanism Simulation**

مثلاً می‌خواهیم اثر احتمالی استفاده از یک روش CRISPR را بررسی کنیم.

در این حالت باید مشخص شود آیا خود روش مداخله می‌تواند تغییر هدف را ایجاد کند و با چه عدم‌قطعیتی.

این دو نباید با هم اشتباه شوند.

### نکته‌ای که به معماری اضافه می‌کنم

یک Model Package برای ژنتیک باید بتواند **Genotype و Cell State** را جداگانه نگه دارد.

چون دو سلول با DNA یکسان می‌توانند بیان ژنی و رفتار متفاوتی داشته باشند.

همچنین تغییر DNA، تغییر RNA و تغییر فعالیت پروتئین سه نوع مداخله متفاوت هستند.

---

## ۲۴. Stem Cell & Regeneration Models

برای پزشکی بازساختی، فقط مدل رشد سلول کافی نیست.

ما باید از مفاهیم زیر پشتیبانی کنیم:

| مفهوم | اهمیت |
|---|---|
| Stem Cell Maintenance | حفظ جمعیت سلول‌های بنیادی |
| Differentiation | تبدیل سلول بنیادی به سلول تخصصی |
| Reprogramming | تغییر برنامه هویتی سلول |
| Cell Migration | رسیدن سلول به موقعیت مناسب |
| Tissue Integration | اتصال سلول جدید به بافت |
| Vascularization | تأمین خون در بافت‌هایی که به آن نیاز دارند |
| Innervation | ارتباط عصبی |
| Extracellular Matrix | ساختار پشتیبان سلول‌ها |
| Immune Response | واکنش ایمنی |
| Tissue Remodeling | سازمان‌دهی مجدد بافت |
| Functional Recovery | بازگشت عملکرد واقعی |
| Developmental Programs | فرایندهای مربوط به شکل‌گیری و رشد اندام |

**بازسازی بافت با زیاد شدن تعداد سلول‌ها یکسان نیست.**

مثلاً اگر در قرنیه تعداد سلول‌ها افزایش پیدا کند، ولی ساختار نهایی شفاف نباشد، عملکرد چشم ممکن است بدتر شود.

اگر در یک عصب سلول جدید تولید شود ولی اتصال عملکردی ایجاد نکند، افزایش تعداد سلول به معنی بازگشت عملکرد نیست.

پس Model Composer باید بتواند **Structural Recovery** و **Functional Recovery** را جداگانه ارزیابی کند.

---

# بخش دهم: Simulation Runtime و اجرای واقعی

## ۲۵. موتور مرکزی چگونه کار می‌کند؟

Runtime نباید تمام معادلات و قوانین را خودش پیاده‌سازی کند.

وظیفه آن مدیریت اجرای مدل‌های مستقل و ترکیبی است.

```text
                 SIMULATION PLAN
                        |
                        v
               Dependency Resolution
                        |
                        v
                Model Initialization
                        |
                        v
                 State Allocation
                        |
                        v
                 Event Scheduling
                        |
                        v
                  Model Execution
                        |
                        v
                Coupling Exchange
                        |
                        v
                Consistency Checks
                        |
                        v
                  State Update
                        |
                        v
                 Observation Export
                        |
                        v
                   Checkpoint
                        |
                        v
                Continue / Complete
```

### Execution Modes

| Mode | کاربرد |
|---|---|
| Single Model | اجرای یک مدل |
| Composite | اجرای چند مدل متصل |
| Multiscale | اجرای مدل‌ها در مقیاس‌های مختلف |
| Parameter Sweep | بررسی مقادیر متعدد پارامتر |
| Ensemble | اجرای مدل‌های جایگزین |
| Replication | تکرار اجرا برای سنجش پایداری |
| Counterfactual | مقایسه شرایط با مداخله و بدون مداخله |
| Sensitivity | شناسایی پارامترهای تأثیرگذار |
| Calibration | تنظیم پارامترها با داده |
| Benchmark | مقایسه با داده مرجع |
| Hybrid | اجرای هم‌زمان مکانیزم‌های ریاضی و ML |
| Adaptive Resolution | تغییر سطح جزئیات بر اساس نیاز و قواعد معتبر |

## ۲۶. State Management

هر شبیه‌سازی باید وضعیت خودش را داشته باشد.

مثلاً:

```text
Simulation State
├── Global Time
├── Active Models
├── Model Internal States
├── Spatial Fields
├── Biological Entities
├── Interactions
├── Environmental Conditions
├── Scheduled Events
├── Random Generator States
└── Checkpoint Metadata
```

باید بتوانیم از یک وضعیت مشخص چند شاخه بسازیم.

مثلاً:

```text
                 BASELINE STATE
                       |
          +------------+------------+
          |            |            |
          v            v            v
       Control     Treatment A  Treatment B
          |            |            |
          v            v            v
       Outcome 1    Outcome 2    Outcome 3
```

برای مقایسه معتبر، علاوه بر وضعیت اولیه یکسان، باید تصادفی بودن شبیه‌سازی و تعداد تکرارها هم مدیریت شود.

### ۲۶.۱. Simulation Time

نباید فرض کنیم یک ثانیه در تمام مدل‌ها معادل یک ثانیه در موتور محاسباتی است.

مثلاً vCornea از Monte Carlo Steps استفاده می‌کند و در تنظیمات منتشرشده هر MCS به یک مقیاس زمانی بیولوژیکی نگاشت شده است.

مدل دیگری ممکن است با ثانیه واقعی یا گام زمانی متغیر اجرا شود.

پس Runtime باید زمان فیزیکی مدل را از تعداد گام‌های محاسباتی جدا کند.

### ۲۶.۲. Geometry & Spatial Reference

باید مختصات مدل‌ها را نیز هماهنگ کنیم.

مثلاً:

- مدل اول دوبعدی است.
- مدل دوم سه‌بعدی است.
- مدل اول یک برش کوچک از بافت است.
- مدل دوم نمای کل اندام را دارد.

نمی‌توان مستقیماً گفت هر سلول دوبعدی معادل یک سلول سه‌بعدی است.

به یک **Spatial Mapping Model** نیاز داریم.

اگر نگاشت معتبر موجود نباشد، اتصال باید محدود یا رد شود.

---

# بخش یازدهم: بررسی ۲۴ سناریو برای Stress-Test معماری

این بخش را برای پیدا کردن ضعف‌های معماری طراحی کردم.

هدف این نیست که بگوییم تمام سناریوها امروز قابل شبیه‌سازی دقیق هستند.

هدف این است که ببینیم **اگر روزی برای این مسائل مدل یا داده مناسب پیدا کردیم، آیا معماری بدون بازنویسی اساسی می‌تواند از آن پشتیبانی کند؟**

سناریوها را به چهار گروه تقسیم می‌کنم.

## ۲۷. گروه اول: شبیه‌سازی و ترکیب مدل‌های زیستی

| # | سناریو | اجزای لازم | مهم‌ترین چالش |
|---|---|---|---|
| ۱ | وارد کردن vCornea و بازتولید نتایج | Model Importer، CompuCell3D Adapter | تفاوت اجرای اصلی و جدید |
| ۲ | افزودن مدل رشد سلول به vCornea | Model Composer، Cell Models | جلوگیری از محاسبه تکراری رشد |
| ۳ | اتصال مدل مولکولی EGF به مدل قرنیه | Molecular + Cellular Coupling | تبدیل مقیاس زمانی و مکانی |
| ۴ | اتصال قرنیه، عدسی و شبکیه | Optical + Tissue + Neural Models | تبدیل وضعیت بافت به ویژگی نوری |
| ۵ | اتصال شبکیه و عصب بینایی | Neural Models، Coupling Engine | ارتباطات عملکردی عصبی |
| ۶ | اتصال چشم و کبد از طریق گردش خون | PBPK، Organ Models | مدل‌سازی انتقال ماده و Feedback |

### سناریو ۱: وارد کردن vCornea

این اولین تست واقعی معماری است.

ما یک مدل منتشرشده داریم که:

- ساختار فایل اختصاصی دارد.
- از CompuCell3D استفاده می‌کند.
- پارامترهای مشخصی دارد.
- در شرایط مختلف نتایج متفاوتی تولید می‌کند.
- فایل‌های خروجی مخصوص خودش را دارد.

سیستم باید ابتدا آن را بدون تغییر علمی اجرا کند.

بعد یک Adapter بسازد که بتواند:

- پارامترها را دریافت کند.
- اجرای مدل را شروع کند.
- وضعیت اجرا را گزارش کند.
- داده‌های خروجی را استخراج کند.
- نتایج را در قالب استاندارد UBSP ارائه دهد.

**قانون:** Adapter نباید بی‌سروصدا قوانین بیولوژیکی مدل را عوض کند.

اگر لازم است تغییری انجام شود، آن تغییر باید در یک Model Version جدید ثبت شود.

مقاله اصلی vCornea یک مدل دوبعدی از اپی‌تلیوم قرنیه را معرفی کرده که رشد، تمایز، مهاجرت و مرگ سلولی را بررسی می‌کند. این نقطه شروع مناسبی برای تست زیرساخت ماست. [PLOS](https://journals.plos.org/ploscompbiol/article?id=10.1371%2Fjournal.pcbi.1013410\&utm_source=chatgpt.com)

### سناریو ۲: اضافه کردن مدل رشد سلول

فرض کن یک مدل جدید برای رشد سلول پیدا کرده‌ایم.

آیا می‌توانیم مستقیم آن را به vCornea اضافه کنیم؟

**نه لزوماً.**

چون vCornea خودش قانون رشد سلولی دارد.

اگر مدل جدید را بدون اصلاح مدل قبلی اضافه کنیم، ممکن است رشد دو بار محاسبه شود.

پس Model Composer باید بتواند:

1. تشخیص دهد دو مدل چه مسئولیت‌هایی دارند.
2. تعارض و هم‌پوشانی را پیدا کند.
3. مشخص کند کدام مسئولیت جایگزین می‌شود.
4. از ایجاد دو مالک برای یک متغیر وضعیت جلوگیری کند.
5. مدل ترکیبی را دوباره ارزیابی کند.

این مسئله را **Model Responsibility Conflict** می‌نامیم.

پیشنهاد می‌کنم در هر Simulation Plan برای متغیرهای اصلی، یک مالک محاسباتی مشخص وجود داشته باشد.

مثلاً تنها یک مدل مسئول محاسبه مقدار نهایی `cell_growth_rate` باشد، مگر اینکه یک قانون ترکیب صریح و معتبر تعریف شده باشد.

### سناریو ۳: اتصال مدل EGF به vCornea

فرض کن یک مدل دقیق‌تر برای اتصال EGF به گیرنده آن داریم.

مدل جدید می‌تواند پاسخ مولکولی را محاسبه کند.

vCornea می‌تواند رفتار بافتی را شبیه‌سازی کند.

برای اتصال آن‌ها باید مشخص کنیم:

- کدام خروجی مولکولی روی کدام پارامتر سلولی اثر می‌گذارد؟
- آیا آن ارتباط در داده‌های واقعی مشاهده شده؟
- آیا مقیاس‌های زمانی مناسب‌اند؟
- آیا خود vCornea قبلاً بخشی از این اثر را به‌صورت ضمنی محاسبه می‌کند؟

اگر مدل اصلی قبلاً اثر EGF را در نرخ رشد لحاظ کرده باشد، اضافه کردن اثر جداگانه می‌تواند باعث Double Counting شود.

در این حالت باید بخشی از مدل قبلی جایگزین شود.

### سناریو ۴: شبیه‌سازی کل چشم

فرض کن می‌خواهیم بررسی کنیم آسیب قرنیه چه تأثیری بر بینایی دارد.

مسیر ممکن است این باشد:

```text
Corneal Tissue Damage
          |
          v
Corneal Geometry & Optical Properties
          |
          v
Light Propagation
          |
          v
Retinal Illumination
          |
          v
Retinal Neural Response
          |
          v
Visual Signal
```

اما vCornea فعلی مستقیماً Optical Properties را محاسبه نمی‌کند.

پس یک ماژول واسط لازم داریم که وضعیت بافت را به ویژگی‌های نوری تبدیل کند.

اگر چنین مدل معتبری وجود نداشته باشد، نمی‌توانیم از خروجی vCornea مستقیماً کیفیت بینایی انسان را نتیجه بگیریم.

این مثال نشان می‌دهد چرا **Missing Model Detection** حیاتی است.

### سناریو ۵: بازسازی عصب بینایی

فرض کن مدل تولید سلول‌های عصبی جدید داریم.

یک مدل دیگر رشد Axon را شبیه‌سازی می‌کند.

مدل سوم انتقال سیگنال عصبی را محاسبه می‌کند.

آیا با ترکیب این سه می‌توانیم بازگشت بینایی را پیش‌بینی کنیم؟

فقط اگر مدل اتصال عملکردی، سازمان‌دهی مدارها و اطلاعات حسی موردنیاز هم وجود داشته باشد.

بنابراین سیستم باید بتواند مدل‌های ناقص را ترکیب کند، اما نتیجه را به‌عنوان پیش‌بینی ناقص گزارش دهد.

### سناریو ۶: چشم و کبد

این سناریو برای آزمودن مدل چنداندامی است.

در یک آزمایش فرضی، ماده‌ای وارد گردش خون می‌شود و غلظت آن بر اثر متابولیسم کبد تغییر می‌کند.

مدل PBPK غلظت ماده را در بخش‌های بدن تخمین می‌زند.

مدل چشم پاسخ بافت به آن غلظت را محاسبه می‌کند.

اگر پاسخ چشم روی وضعیت سیستمیک اثر بگذارد، Runtime باید بازخورد را هم مدیریت کند.

این سناریو بدون نیاز به شبیه‌سازی تک‌تک سلول‌های کبد قابل بررسی است، به شرط اینکه مدل‌های خلاصه‌شده برای سؤال موردنظر معتبر باشند.

---

## ۲۸. گروه دوم: ژنتیک، سلول بنیادی و پزشکی بازساختی

| # | سناریو | قابلیت موردنیاز | چالش اصلی |
|---|---|---|---|
| ۷ | تغییر فعالیت یک ژن در سلول قرنیه | Genetic Intervention | ارتباط ژن با رفتار سلول |
| ۸ | بررسی اثر یک تغییر ژنتیکی واقعی | Variant Interpretation | تفاوت اثر DNA، RNA و پروتئین |
| ۹ | بررسی دو تغییر ژنتیکی هم‌زمان | Combinatorial Perturbation | تعامل غیرخطی دو تغییر |
| ۱۰ | تبدیل سلول بنیادی به سلول تخصصی | Differentiation Models | صحت مسیر تمایز |
| ۱۱ | جایگزینی سلول‌های ازبین‌رفته شبکیه | Cell Transplantation Simulation | جای‌گیری و عملکرد سلول |
| ۱۲ | شبیه‌سازی شکل‌گیری مجدد یک اندام | Developmental Models | هماهنگی رشد، ساختار و عملکرد |

### سناریو ۷: تغییر فعالیت ژن

کاربر می‌خواهد بررسی کند:

«اگر فعالیت ژن X در یک نوع سلول بیشتر شود، آیا سرعت ترمیم بافت تغییر می‌کند؟»

سیستم:

1. مدل مربوط به آن نوع سلول را پیدا می‌کند.
2. داده‌های Perturbation مناسب را جست‌وجو می‌کند.
3. مدل پاسخ ژنی را انتخاب می‌کند.
4. تغییر فرضی را اعمال می‌کند.
5. پاسخ سلولی را تخمین می‌زند.
6. در صورت وجود Coupling معتبر، اثر را به مدل بافت منتقل می‌کند.
7. نتایج و عدم‌قطعیت را گزارش می‌کند.

اگر ارتباط ژن و رشد سلول معلوم نباشد، سیستم نباید آن را اختراع کند.

### سناریو ۸: تغییر واقعی DNA

این مورد با سناریوی قبل متفاوت است.

ممکن است یک تغییر در DNA:

- هیچ اثر قابل اندازه‌گیری نداشته باشد.
- میزان بیان RNA را تغییر دهد.
- ساختار پروتئین را تغییر دهد.
- فقط در نوع خاصی از سلول اثر بگذارد.
- در شرایط خاص بیماری اثر خود را نشان دهد.

پس یک Variant Effect Model ممکن است لازم باشد.

همچنین باید مشخص شود تغییر در یک نسخه ژن رخ می‌دهد یا هر دو نسخه، و آیا تمام سلول‌های هدف همان تغییر را دارند یا خیر.

### سناریو ۹: تغییر دو ژن هم‌زمان

فرض کن:

```text
Gene A Change -> 10% Cell Growth Increase

Gene B Change -> 15% Cell Growth Increase
```

آیا تغییر هم‌زمان آن‌ها باعث افزایش ۲۵٪ رشد می‌شود؟

**خیر، لزوماً.**

ممکن است دو ژن در یک مسیر مشترک باشند.

ممکن است یکی اثر دیگری را مهار کند.

ممکن است اثر ترکیبی حتی منفی شود.

این یک مسئله **Epistasis و Genetic Interaction** است.

بنابراین نباید اثر دو مداخله را بدون مدل معتبر با هم جمع کنیم.

حتی توسعه‌دهندگان GEARS تأکید کرده‌اند که پیش‌بینی ترکیب مداخلات از مدلی که فقط با داده مداخلات تکی آموزش دیده، الزاماً قابل‌اعتماد نیست. [GitHub](https://github.com/snap-stanford/GEARS?utm_source=chatgpt.com)

### سناریو ۱۰: سلول بنیادی

فرض کن می‌خواهیم بررسی کنیم آیا یک Stem Cell می‌تواند به نوع مشخصی از سلول شبکیه تبدیل شود.

مدل باید بتواند وضعیت‌های مختلف سلول را نمایش دهد.

اما صرف اینکه یک مدل ML وضعیت ژنی سلول نهایی را پیش‌بینی کند کافی نیست.

باید مواردی مانند:

- بقای سلول
- ثبات هویت
- عملکرد تخصصی
- جای‌گیری در بافت
- تعامل با سلول‌های مجاور

در صورت وجود مدل‌های معتبر بررسی شوند.

### سناریو ۱۱: جایگزینی سلول در شبکیه

فرض کن سلول‌های حساس به نور از بین رفته‌اند.

ما سلول‌های جدیدی را در مدل وارد می‌کنیم.

سیستم باید بررسی کند:

- آیا سلول‌ها زنده می‌مانند؟
- آیا در محل مناسب قرار می‌گیرند؟
- آیا با بافت اطراف سازگارند؟
- آیا ارتباط عصبی شکل می‌گیرد؟
- آیا فعالیت الکتریکی مؤثر ایجاد می‌شود؟

اینجا Cell Count به‌تنهایی معیار موفقیت نیست.

**Functional Integration** معیار مهم‌تری است.

### سناریو ۱۲: ساخت مجدد اندام

فرض کن می‌خواهیم در آینده امکان رشد مجدد یک ساختار پیچیده را بررسی کنیم.

این نیازمند مدل‌های Developmental Biology، Mechanical Growth، Cell Differentiation، Vascularization و Neural Integration است.

وجود مدل تک‌تک اجزا به‌تنهایی تضمین نمی‌کند که بتوان رشد کامل اندام را شبیه‌سازی کرد.

در این سناریو، سیستم باید بتواند رشد هندسه و ایجاد اجزای جدید را نیز در طول زمان مدل کند.

---

## ۲۹. گروه سوم: داده، یادگیری و توسعه مدل‌های جدید

| # | سناریو | قابلیت لازم | ریسک |
|---|---|---|---|
| ۱۳ | فقط تصاویر میکروسکوپی داریم | Imaging Pipeline | ادعای نادرست درباره دینامیک |
| ۱۴ | فقط داده Single-Cell RNA داریم | Omics Pipeline | تبدیل اشتباه Expression به رفتار |
| ۱۵ | تصاویر Time-lapse داریم | Dynamic Model Learning | خطای اندازه‌گیری حرکت و رشد |
| ۱۶ | دو دیتاست از انسان و موش داریم | Cross-Species Mapping | انتقال نادرست |
| ۱۷ | چند منبع داده متناقض داریم | Evidence Conflict Management | انتخاب دلخواه داده |
| ۱۸ | مدل آماده نداریم ولی داده زیاد داریم | Model Construction Engine | نبود اطلاعات کافی برای شناسایی مدل |

### سناریو ۱۳: فقط تصویر داریم

فرض کن ده هزار تصویر از سلول‌های قرنیه داریم.

سیستم می‌تواند:

- سلول‌ها را شناسایی کند.
- شکل و تعداد آن‌ها را اندازه بگیرد.
- وضعیت‌های ظاهری را طبقه‌بندی کند.
- ساختار آماری بافت را بررسی کند.

اما نمی‌تواند فقط از همین تصاویر ثابت، قانون دقیق تقسیم سلول را به‌طور قطعی به دست آورد.

برای شبیه‌سازی دینامیک به داده زمانی یا فرضیات مکانیزمی اضافی نیاز داریم.

**بنابراین Model Construction Engine باید بتواند بگوید: داده موجود برای هدف موردنظر کافی نیست.**

### سناریو ۱۴: داده RNA داریم

فرض کن یک دیتاست Single-Cell RNA داریم.

می‌توانیم وضعیت بیان ژن و جمعیت‌های سلولی را بررسی کنیم.

اما RNA Expression مستقیماً معادل مقدار پروتئین یا فعالیت سلول نیست.

پس نباید بدون مدل واسط، مقدار RNA را به سرعت تقسیم سلول تبدیل کنیم.

ممکن است مدل ژنی فقط برای پیش‌بینی Transcriptomic State معتبر باشد.

### سناریو ۱۵: Time-lapse Imaging

فرض کن در طول ۴۸ ساعت، هر چند دقیقه از سلول‌ها تصویر داریم.

این داده می‌تواند برای مدل کردن:

- حرکت سلول
- تغییر اندازه
- تقسیم
- مرگ
- الگوهای جمعی

مفید باشد.

اما الگوریتم Tracking ممکن است دو سلول را اشتباه تشخیص دهد.

بنابراین خطای Measurement Pipeline باید وارد ارزیابی مدل شود.

### سناریو ۱۶: داده انسان و موش

فرض کن برای موش داده Perturbation زیادی داریم، ولی برای انسان داده محدود است.

می‌توانیم انتقال دانش را بررسی کنیم.

اما:

- ژن‌های متناظر ممکن است عملکرد متفاوتی داشته باشند.
- ویژگی‌های سلول‌ها ممکن است متفاوت باشند.
- نرخ رشد و ترمیم ممکن است تغییر کند.
- مسیرهای پیام‌رسانی می‌توانند تفاوت داشته باشند.

پس Species Mapping باید مبتنی بر شناسه‌های ژنی، روابط ارتولوژی و شواهد زیستی باشد.

صرف یکسان بودن نام ژن کافی نیست.

### سناریو ۱۷: داده‌های متناقض

فرض کن دو مقاله درباره یک پارامتر نتایج متفاوتی گزارش کرده‌اند.

نباید سیستم خودکار یکی را حذف کند.

باید بررسی کند:

- شرایط آزمایش
- نوع سلول
- گونه
- تعداد نمونه
- روش اندازه‌گیری
- کیفیت داده
- تفاوت زمانی و مکانی

ممکن است هر دو نتیجه در شرایط خودشان درست باشند.

اگر اختلاف حل نشد، باید چند فرضیه یا چند مجموعه پارامتر را حفظ کنیم.

### سناریو ۱۸: داده زیاد، اما مدل نداریم

این یکی از مهم‌ترین سناریوهاست.

فرض کن یک مجموعه بزرگ Spatial Transcriptomics از یک بافت داریم.

ممکن است بتوانیم ساختار فضایی و جمعیت‌های سلولی را بازسازی کنیم.

ولی برای ساخت شبیه‌ساز رفتار آینده بافت، هنوز به اطلاعات دینامیکی و مکانیزمی نیاز داشته باشیم.

در این حالت سیستم باید بتواند:

1. مدل‌های قابل ساخت را پیشنهاد دهد.
2. متغیرهای قابل شناسایی را مشخص کند.
3. پارامترهای نامشخص را گزارش کند.
4. داده موردنیاز برای رفع ابهام را پیشنهاد دهد.
5. از ساخت مدل غیرقابل‌دفاع جلوگیری کند.

این قابلیت را **Model Identifiability & Feasibility Analysis** می‌نامیم.

---

## ۳۰. گروه چهارم: تعاملات پیچیده و شکست‌های سیستم

| # | سناریو | نیاز معماری | رفتار صحیح |
|---|---|---|---|
| ۱۹ | تغییر فشار چشم و اثر روی عصب بینایی | Mechanics + Neural Coupling | بررسی اثر مکانیکی و عصبی |
| ۲۰ | محیط بیرونی تغییر می‌کند | Environmental Simulation | اعمال محرک در زمان و مکان مناسب |
| ۲۱ | دو مدل متصل نتایج ناپایدار می‌دهند | Numerical Validation | توقف، بررسی و گزارش خطا |
| ۲۲ | نسخه جدید مدل، مدل دیگری را خراب می‌کند | Dependency Tracking | Revalidation و Rollback |
| ۲۳ | یک دیتاست اصلی بعداً نامعتبر می‌شود | Provenance & Impact Analysis | شناسایی خروجی‌های وابسته |
| ۲۴ | مدل خارج از شرایط اعتبارش استفاده می‌شود | Applicability Engine | رد اجرا یا اجرای اکتشافی با هشدار صریح |

### سناریو ۱۹: فشار چشم و آسیب عصبی

برای بررسی Glaucoma، مدل‌های مربوط به:

- فشار داخل چشم
- ویژگی‌های مکانیکی بافت
- وضعیت عصب بینایی
- بقای سلول‌های عصبی
- عملکرد بینایی

می‌توانند به یکدیگر متصل شوند.

اما افزایش فشار تنها عامل بیماری نیست و اثر آن در افراد مختلف متفاوت است.

پس مدل باید بتواند عوامل دیگر و عدم‌قطعیت را نیز در نظر بگیرد.

### سناریو ۲۰: تغییر محیط بیرونی

فرض کن دمای محیط تغییر می‌کند.

یا میزان اکسیژن کاهش پیدا می‌کند.

یا بافت در معرض یک ماده شیمیایی قرار می‌گیرد.

این تغییرات باید از طریق **Environment Model** وارد Simulation شوند.

محیط نباید صرفاً یک Input ثابت در ابتدای آزمایش باشد.

گاهی شرایط محیطی در طول زمان تغییر می‌کنند.

مثلاً:

```text
t = 0h      Normal Oxygen
t = 2h      Reduced Oxygen
t = 4h      Oxygen Restored
t = 24h     Measure Cell Response
```

Runtime باید بتواند چنین Eventهایی را اجرا کند.

### سناریو ۲۱: ناپایداری اتصال

دو مدل به‌تنهایی درست کار می‌کنند.

اما هنگام اتصال، غلظت یک ماده به مقادیر غیرواقعی می‌رسد.

سیستم نباید صرفاً چون برنامه Crash نکرده نتیجه را معتبر بداند.

باید:

- قیود فیزیکی را بررسی کند.
- خطای عددی را اندازه بگیرد.
- رفتار در گام‌های زمانی کوچک‌تر را مقایسه کند.
- مدل‌های درگیر را مشخص کند.
- نتیجه نامعتبر را از خروجی علمی جدا کند.

### سناریو ۲۲: بهبود یک مدل، خراب شدن مدل دیگر

فرض کن مدل EGF را بهبود داده‌ایم.

نسخه جدید در قرنیه بهتر عمل می‌کند.

اما مدل پوست که از آن استفاده می‌کند، نتیجه ضعیف‌تری تولید می‌کند.

در این حالت باید اجازه بدهیم:

```text
Cornea -> EGF Model v2

Skin   -> EGF Model v1
```

هر مدل وابسته می‌تواند نسخه مناسب خودش را نگه دارد.

به‌روزرسانی یک Library مشترک نباید تمام مصرف‌کننده‌ها را مجبور به مهاجرت کند.

### سناریو ۲۳: داده نامعتبر

فرض کن یک Dataset بعداً اصلاح یا حذف شود.

سیستم باید بتواند این مسیر را دنبال کند:

```text
Invalidated Dataset
        |
        v
Affected Training Runs
        |
        v
Affected Model Versions
        |
        v
Affected Simulations
        |
        v
Affected Scientific Claims
        |
        v
Revalidation Required
```

تمام تاریخچه حفظ می‌شود.

اما نتایجی که اعتبارشان تحت تأثیر قرار گرفته باید علامت‌گذاری شوند.

### سناریو ۲۴: استفاده خارج از محدوده اعتبار

فرض کن مدل مخصوص سلول‌های قرنیه سالم است.

یک پژوهشگر می‌خواهد آن را برای بافتی با بیماری شدید استفاده کند.

سیستم باید مشخص کند که آیا آن شرایط در دامنه اعتبار مدل قرار دارند یا نه.

اگر شواهد کافی نداریم، خروجی نمی‌تواند همان سطح اعتماد را داشته باشد.

برای فعالیت اکتشافی ممکن است اجرای محدود و برچسب‌دار مجاز باشد.

اما نباید نتیجه آن به‌عنوان پیش‌بینی تأییدشده نمایش داده شود.

---

## ۳۱. نتیجه Stress-Test

این ۲۴ سناریو نشان می‌دهند که معماری اولیه باید با چند قابلیت تکمیل شود.

| قابلیت تکمیلی | چرا اضافه شد؟ |
|---|---|
| Model Responsibility Registry | جلوگیری از محاسبه تکراری یک مکانیزم |
| Model Feasibility Analyzer | تشخیص اینکه داده برای ساخت مدل کافی است یا نه |
| Numerical Coupling Validator | جلوگیری از خطای اتصال مدل‌ها |
| Cross-Species Mapping | جلوگیری از انتقال نادرست داده حیوان به انسان |
| Dynamic Environment Engine | شبیه‌سازی محرک‌های متغیر در زمان |
| Missing Knowledge Registry | ثبت مکانیزم‌های ناشناخته |
| Model Identifiability Analysis | تشخیص پارامترهای غیرقابل شناسایی |
| Dependency Impact Analysis | شناسایی آثار تغییر مدل و داده |
| Context-Specific Applicability | جلوگیری از استفاده نامعتبر از مدل |
| Model Responsibility Ownership | مشخص کردن مالک محاسباتی هر State |
| Structural & Functional Evaluation | تفکیک ساختار سالم از عملکرد سالم |
| Scientific Conflict Registry | ثبت داده‌ها و شواهد متناقض |

این‌ها به‌نظرم مهم‌ترین تکمیل‌های معماری نسبت به نسخه قبلی هستند.

---

# بخش دوازدهم: Scientific Validation & Trust

## ۳۲. چطور بفهمیم مدل‌ها واقعاً قابل اعتمادند؟

به نظرم این بخش باید یکی از مهم‌ترین اجزای پلتفرم باشد.

اگر مدل‌ها از منابع مختلف وارد شوند، نمی‌توانیم فرض کنیم تمام آن‌ها به یک اندازه دقیق هستند.

حتی مدلی که در مقاله معتبر منتشر شده ممکن است فقط در شرایط محدودی درست عمل کند.

بنابراین برای هر مدل، یک **Scientific Trust Profile** می‌سازیم.

### پنج معیار اصلی اعتماد

| معیار | سؤال |
|---|---|
| Verification | آیا محاسبات و پیاده‌سازی درست هستند؟ |
| Validation | آیا نتایج با مشاهدات مستقل واقعی سازگارند؟ |
| Applicability | آیا مدل برای شرایط فعلی مناسب است؟ |
| Robustness | آیا نتیجه در برابر تغییرات کوچک و نویز پایدار است؟ |
| Reproducibility | آیا دیگران می‌توانند نتایج را بازتولید کنند؟ |

این معیارها باید برای هر Model Version و Context ثبت شوند.

### مثال

فرض کن vCornea برای سه سؤال استفاده می‌شود:

| سؤال | وضعیت پیشنهادی اعتبار |
|---|---|
| بازتولید چرخه طبیعی سلول‌های سطح قرنیه | دارای شواهد منتشرشده، نیازمند بازتولید و بررسی مستقل |
| بررسی تغییرات زمان ترمیم با پارامترهای جدید | نیازمند اعتبارسنجی اختصاصی |
| پیش‌بینی بازگشت بینایی بعد از آسیب عصب بینایی | خارج از دامنه مدل |

پس نباید به vCornea یک عدد کلی مثل `Accuracy = 90%` بدهیم.

مدل ممکن است برای یک Endpoint مناسب و برای Endpoint دیگر نامناسب باشد.

### استفاده از استانداردهای موجود

چارچوب ASME V&V 40 و راهنمای FDA درباره اعتبار مدل‌های محاسباتی پزشکی بر یک اصل مهم تأکید دارند: میزان شواهد لازم به **کاربرد مدل و پیامد اشتباه بودن تصمیم** بستگی دارد.

راهنمای FDA به‌طور مشخص برای مدل‌های محاسباتی مکانیزمی در حوزه تجهیزات پزشکی نوشته شده است، نه تمام مدل‌های زیستی یا ML. بنابراین از اصول آن برای طراحی Trust Framework استفاده می‌کنیم، نه اینکه آن را مستقیماً قانون تمام مدل‌های پلتفرم بدانیم. [ASME](https://www.asme.org/codes-standards/find-codes-standards/assessing-credibility-of-computational-modeling-through-verification-and-validation-application-to-medical-devices?utm_source=chatgpt.com)

---

## ۳۳. Independent Validation Engine

این موتور باید مستقل از Model Improvement Engine باشد.

```text
             CANDIDATE MODEL
                    |
                    v
           Technical Verification
                    |
                    v
           Scientific Consistency
                    |
                    v
           Independent Test Data
                    |
                    v
            Error Measurement
                    |
                    v
           Uncertainty Assessment
                    |
                    v
           Applicability Review
                    |
                    v
             Validation Report
                    |
                    v
           Accept / Reject / Review
```

### سه سطح Benchmark

**A. Synthetic Benchmark**

داده‌هایی که پاسخ درست آن‌ها را می‌دانیم.

برای تست الگوریتم‌های کالیبراسیون، شناسایی پارامتر و قابلیت‌های Runtime مناسب‌اند.

**B. Published Scientific Benchmark**

مدل‌ها و داده‌های منتشرشده که امکان مقایسه دارند.

برای بررسی بازتولید نتایج مطالعات مناسب‌اند.

**C. Independent Experimental Benchmark**

داده‌های واقعی که برای ساخت یا تنظیم مدل استفاده نشده‌اند.

برای ارزیابی قابلیت پیش‌بینی اهمیت بیشتری دارند.

در آینده می‌توان از آزمایش‌های آینده‌نگر نیز استفاده کرد.

یعنی ابتدا پیش‌بینی را ثبت کنیم و سپس با نتیجه آزمایش جدید مقایسه کنیم.

---

## ۳۴. معیارهای ارزیابی

باید میان کیفیت نرم‌افزار و کیفیت علمی تفاوت بگذاریم.

### معیارهای مهندسی

| Metric | هدف |
|---|---|
| Execution Success Rate | پایداری اجرا |
| Reproducibility | بازتولید نتایج |
| Runtime | زمان محاسبه |
| Compute Cost | مصرف منابع |
| Numerical Error | خطای عددی |
| Memory Usage | مصرف حافظه |
| Interface Compatibility | سازگاری مدل‌ها |

### معیارهای علمی

| Metric | هدف |
|---|---|
| Prediction Error | فاصله خروجی با داده واقعی |
| Calibration Quality | کیفیت تخمین پارامتر |
| Uncertainty Calibration | سازگاری عدم‌قطعیت پیش‌بینی‌شده با خطای واقعی |
| Out-of-Distribution Performance | عملکرد خارج از داده آموزشی |
| Mechanistic Consistency | سازگاری با مکانیزم‌های شناخته‌شده |
| Biological Constraint Violations | تعداد یا شدت نقض قیود علمی |
| Cross-Context Generalization | عملکرد در شرایط متفاوت |
| Intervention Effect Error | خطای پیش‌بینی پاسخ به مداخله |

در هر Model Package باید معیارهای مناسب همان مدل تعریف شوند.

مثلاً F1 Score برای یک مدل Classification می‌تواند مناسب باشد، ولی برای سنجش رشد زمانی بافت کافی نیست.

---

## ۳۵. Uncertainty Engine

یکی از اشتباهات خطرناک این است که تمام پیش‌بینی‌ها را با یک Confidence Score نمایش دهیم.

من پیشنهاد می‌کنم عدم‌قطعیت را به شش نوع تقسیم کنیم.

| نوع | معنی | مثال |
|---|---|---|
| Parameter | مقدار دقیق پارامتر را نمی‌دانیم | نرخ تقسیم سلول |
| Structural | ممکن است خود معادله ناقص باشد | مکانیزم رشد سلول |
| Measurement | داده آزمایشگاهی خطا دارد | شمارش سلول از تصویر |
| Biological | موجودات یا سلول‌ها واقعاً متفاوت‌اند | تفاوت افراد |
| Coupling | ارتباط مدل‌ها نامطمئن است | اتصال مدل ژن به مدل بافت |
| Extrapolation | مدل را خارج از داده معتبر استفاده می‌کنیم | انتقال موش به انسان |

در هر Prediction Report باید مشخص شود کدام عدم‌قطعیت‌ها اندازه‌گیری شده‌اند و کدام هنوز ناشناخته‌اند.

### مثال

فرض کن شبیه‌ساز زمان ترمیم را ۴ روز پیش‌بینی می‌کند.

خروجی مناسب ممکن است چنین باشد:

```text
Predicted Healing Time:
    4.0 days

Estimated Parameter Uncertainty:
    3.5 - 4.8 days

Model Structure Uncertainty:
    Not quantified

Validation Context:
    Limited experimental conditions

Clinical Reliability:
    Not established
```

این مثال فرضی است.

فاصله ۳٫۵ تا ۴٫۸ روز فقط عدم‌قطعیت مشخص‌شده را نشان می‌دهد، نه تمام عدم‌قطعیت‌های ممکن.

این تفاوت جلوی اعتماد کاذب به پیش‌بینی را می‌گیرد.

---

# بخش سیزدهم: Model Learning & Evolution

## ۳۶. مدل‌ها چگونه باید تکامل پیدا کنند؟

یک بخش مهم از هدف تو این است که سیستم فقط مدل‌های موجود را اجرا نکند، بلکه بتواند با داده‌های جدید آن‌ها را بهتر کند.

برای این کار پیشنهاد می‌کنم **Model Evolution Engine** از هفت نوع عملیات پشتیبانی کند.

| نوع تغییر | مثال | میزان تغییر |
|---|---|---|
| Parameter Update | تغییر نرخ تقسیم سلول | محدود |
| Equation Refinement | اصلاح معادله رشد | متوسط تا زیاد |
| Mechanism Addition | اضافه کردن سیستم ایمنی | زیاد |
| Model Replacement | جایگزینی مدل رشد قدیمی | زیاد |
| Coupling Improvement | اصلاح ارتباط دو مدل | متوسط تا زیاد |
| Resolution Upgrade | تبدیل مدل ساده به مدل دقیق‌تر | وابسته به مدل |
| New Model Construction | ساخت مدل برای مکانیزم ناشناخته | زیاد |

هر تغییر باید نسخه مستقل داشته باشد.

نباید مستقیماً مدل اصلی را Overwrite کنیم.

### چرخه تکامل

```text
                   MODEL v1
                      |
                      v
                Baseline Results
                      |
                      v
                  New Dataset
                      |
                      v
                Error / Gap Analysis
                      |
                      v
               Improvement Proposal
                      |
                      v
                   MODEL v2
                      |
                      v
              Independent Evaluation
                      |
            +---------+---------+
            |         |         |
            v         v         v
         Better     Worse    Inconclusive
            |         |         |
            v         v         v
         Promote    Reject     Review
```

### قانون مهم

**مدل جدید فقط وقتی بهتر است که روی داده‌های مناسب و مستقل، در معیارهای مرتبط، بهبود قابل دفاع نشان دهد و محدودیت‌های علمی را نقض نکند.**

ممکن است مدل جدید در یک شرایط بهتر و در شرایط دیگری بدتر باشد.

در این حالت دو نسخه را نگه می‌داریم.

---

## ۳۷. آیا از Reinforcement Learning استفاده کنیم؟

بله، ولی نه برای تمام مسائل.

سه کاربرد را جدا می‌کنم.

### A. Parameter Optimization

هدف: پیدا کردن پارامترهایی که مدل را به داده واقعی نزدیک‌تر می‌کنند.

در بسیاری از مسائل می‌توان با روش‌هایی مثل Bayesian Optimization شروع کرد.

### B. Experiment Selection

هدف: انتخاب آزمایش بعدی که بیشترین اطلاعات مفید را تولید می‌کند.

اینجا Active Learning، Bayesian Experimental Design یا روش‌های مشابه مناسب‌اند.

### C. Sequential Control

هدف: یادگیری یک سیاست برای انتخاب مداخله در طول زمان.

مثلاً در یک مدل معتبر، یک Agent باید از میان مجموعه‌ای از اقدامات مجاز، اقدام بعدی را انتخاب کند.

اینجا RL ممکن است مفید باشد.

اما RL برای حل مسئله سوم به یک محیط آموزشی مناسب نیاز دارد.

اگر Simulator اشتباه باشد، RL می‌تواند راهی برای سوءاستفاده از اشتباهات آن پیدا کند.

بنابراین پیشنهاد من:

**در MVP از Calibration، Sensitivity Analysis و Bayesian Optimization شروع کنیم. RL را برای مسائل Sequential و دارای Benchmark معتبر اضافه کنیم.**

---

## ۳۸. Active Model Development

یک قابلیت پیشرفته‌تر که ارزش اضافه کردن دارد:

سیستم بتواند تشخیص دهد **کدام داده جدید بیشترین کمک را به بهبود یک مدل می‌کند**.

مثلاً دو پارامتر نامطمئن داریم:

- سرعت تقسیم سلول
- سرعت مهاجرت سلول

هر دو ممکن است روی زمان بسته شدن زخم تأثیر بگذارند.

اگر فقط زمان بسته شدن زخم را اندازه بگیریم، شاید نتوانیم اثر آن‌ها را جدا کنیم.

اما اگر حرکت سلول‌ها را در طول زمان هم اندازه بگیریم، ابهام کمتر شود.

پس سیستم می‌تواند پیشنهاد دهد:

«برای بهبود مدل، اندازه‌گیری حرکت سلول در زمان‌های مختلف ارزش اطلاعاتی بیشتری از اندازه‌گیری مجدد زمان نهایی ترمیم دارد.»

این قابلیت بر اساس **Identifiability، Sensitivity و Expected Information Gain** طراحی می‌شود.

اما نباید از ابتدا فرض کنیم سیستم به‌طور خودکار ارزش واقعی تمام آزمایش‌ها را می‌داند.

در ابتدا باید این روش را روی مسائل محدود با پاسخ شناخته‌شده ارزیابی کنیم.

---

# بخش چهاردهم: تاریخچه، نسخه‌بندی و Provenance

## ۳۹. پنج نوع تاریخچه مستقل

پیشنهاد قبلی را حفظ می‌کنم و آن را دقیق‌تر می‌کنم.

| تاریخچه | چه چیزی ثبت می‌شود؟ |
|---|---|
| Model History | نسخه‌ها و تغییرات مدل |
| Dataset History | منابع و تغییرات داده |
| Simulation History | تمام اجراها و شرایطشان |
| Validation History | ارزیابی‌ها و نتیجه آن‌ها |
| Scientific Decision History | تصمیم درباره پذیرش یا رد مدل |

یک شبیه‌سازی باید قابل ردیابی باشد.

مثلاً:

```text
Simulation Run: RUN-1024
        |
        ├── Model Package: Cornea v2.1
        |
        ├── Coupling Model: EGF v1.3
        |
        ├── Dataset: DATA-48
        |
        ├── Parameters: PARAM-22
        |
        ├── Engine: CompuCell3D 4.7.0
        |
        ├── Environment: Container Digest
        |
        ├── Random Seed: 12345
        |
        ├── Results: ARTIFACT-95
        |
        └── Validation: REPORT-17
```

این شناسه‌ها نمونه‌اند.

تمام اطلاعات باید نسخه‌دار باشند.

### استانداردهای مناسب

**PROV-O** برای نمایش منشأ داده و فعالیت‌ها مناسب است.

**RO-Crate** برای بسته‌بندی خروجی‌های تحقیقاتی به همراه Metadata، نرم‌افزار، منابع و تاریخچه مفید است.

هر دو استاندارد امکان نگهداری ارتباط میان داده، فعالیت و عامل تولیدکننده را فراهم می‌کنند. [W3C](https://www.w3.org/TR/prov-o/?utm_source=chatgpt.com)

---

## ۴۰. Scientific Claim Registry

این مفهوم در Blueprint پروژه Research Lab وجود دارد و باید رابط سازگار با آن در Simulator نیز داشته باشیم.

اما UBSP نباید خودش یک Research Knowledge Management Platform کامل و تکراری بسازد.

در UBSP صرفاً ادعاهای مربوط به مدل و شبیه‌سازی ثبت می‌شوند.

مثلاً:

```text
Claim:
    Increasing parameter X reduced
    simulated wound closure time by 12%.

Evidence Type:
    Simulation Result

Model:
    Cornea v2.1

Conditions:
    Context C

Independent Validation:
    Not available

Clinical Evidence:
    None
```

این یک نتیجه شبیه‌سازی فرضی است.

سیستم نباید آن را تبدیل کند به:

«افزایش X باعث ترمیم سریع‌تر قرنیه انسان می‌شود.»

این ادعا نیازمند شواهد مستقلی است که مدل به‌تنهایی تولید نمی‌کند.

---

## ۴۱. Model Dependency Graph

این یکی از مهم‌ترین اجزای مدیریت تغییر است.

```text
               EGF MODEL v1
                  /       \
                 /         \
                v           v
         CORNEA MODEL    SKIN MODEL
                |           |
                v           v
          EYE SYSTEM    SKIN SYSTEM
                |
                v
          HUMAN MODEL
```

اگر EGF Model تغییر کند، سیستم باید مدل‌های وابسته را پیدا کند.

اما لازم نیست تمام شبیه‌سازی‌های جهان را دوباره اجرا کند.

بهتر است Dependency Impact Analyzer بررسی کند:

- کدام مدل‌ها واقعاً از قسمت تغییرکرده استفاده می‌کنند؟
- کدام Endpointها تحت تأثیرند؟
- کدام Benchmarkها باید تکرار شوند؟
- کدام نتایج قبلی نیازمند بازبینی هستند؟

این امکان هزینه Revalidation را کاهش می‌دهد.

---

# بخش پانزدهم: Governance، امنیت و محدودیت‌های علمی

## ۴۲. چه تغییراتی می‌توانند خودکار باشند؟

پیشنهاد می‌کنم چهار سطح اختیار تعریف کنیم.

| سطح | تغییرات | اختیار سیستم |
|---|---|---|
| L0 | امنیت، Audit، مجوزها و قواعد علمی بنیادی | غیرقابل تغییر خودکار |
| L1 | تعریف Ground Truth و معیارهای پذیرش | نیازمند تأیید مستقل |
| L2 | ساختار مدل، معادلات و روش یادگیری | پیشنهاد و آزمایش در Sandbox |
| L3 | اجرای آزمایش‌های مجاز و تغییر پارامتر در محدوده مشخص | قابل خودکارسازی |

مثلاً:

تغییر مقدار یک پارامتر در آزمایش محاسباتی می‌تواند خودکار باشد.

اما تغییر قانون اصلی رشد سلول باید یک Candidate Model جدید ایجاد کند.

تغییر مجموعه Benchmark مستقل نیز نباید توسط همان Agent که مدل را بهینه می‌کند انجام شود.

این تفکیک از خودتأییدکنندگی جلوگیری می‌کند.

## ۴۳. امنیت مدل‌های خارجی

فرض کن یک Repository جدید پیدا می‌کنیم.

سیستم نباید مستقیم کد آن را روی سرور اصلی اجرا کند.

باید:

- Dependencyها را بررسی کند.
- کد را در محیط ایزوله اجرا کند.
- دسترسی شبکه و فایل را محدود کند.
- منابع CPU، GPU و حافظه را محدود کند.
- فایل‌های ورودی و خروجی را کنترل کند.
- نسخه کد و Container را ثبت کند.

این موضوع هم برای امنیت زیرساخت مهم است و هم برای بازتولید آزمایش‌ها.

## ۴۴. مجوزهای استفاده از کد و داده

یک مسئله حقوقی مهم وجود دارد.

**Public Repository الزاماً به معنی مجوز آزاد برای تغییر و استفاده تجاری نیست.**

مثلاً در بررسی Repository پروژه vCornea، فایل License مستقلی در ریشه Repository مشاهده نکردم.

مقاله علمی آن تحت مجوز Open Access منتشر شده، اما این موضوع به‌تنهایی مجوز استفاده تجاری از تمام کد را تعیین نمی‌کند.

پس قبل از Fork تجاری، توزیع یا تغییر کد باید مجوز آن را روشن کنیم.

برای مدل‌های دیگر هم همین موضوع اهمیت دارد.

مثلاً PK-Sim تحت GPLv2 منتشر شده، درحالی‌که Chaste مجوز BSD دارد. این تفاوت در روش ادغام و توزیع نرم‌افزار مهم است. [GitHub](https://github.com/open-systems-pharmacology/pk-sim?utm_source=chatgpt.com)

به همین دلیل هر Model Package باید License Metadata داشته باشد.

---

# بخش شانزدهم: ساختار نهایی Repository

## ۴۵. آیا یک Repository باشد یا چند Repository؟

**پیشنهاد من: یک Monorepo مستقل برای UBSP، با قابلیت استخراج Packageها به Repositoryهای جداگانه در آینده.**

چرا؟

در مراحل اولیه تعداد اجزا زیاد نیست، ولی وابستگی‌هایشان دائماً تغییر می‌کند.

اگر از ابتدا ۲۰ Repository جدا بسازیم، Version Management، تست Integration و توسعه قراردادهای مشترک پیچیده می‌شود.

بنابراین ساختار زیر را پیشنهاد می‌کنم.

```text
universal-bio-simulator/
│
├── apps/
│   ├── api/
│   ├── dashboard/
│   ├── cli/
│   └── worker/
│
├── packages/
│   │
│   ├── core/
│   │   ├── contracts/
│   │   ├── types/
│   │   ├── units/
│   │   ├── coordinates/
│   │   └── events/
│   │
│   ├── biology/
│   │   ├── ontology/
│   │   ├── anatomy/
│   │   ├── entities/
│   │   ├── mechanisms/
│   │   ├── interactions/
│   │   └── knowledge_graph/
│   │
│   ├── model_registry/
│   │   ├── models/
│   │   ├── capabilities/
│   │   ├── versions/
│   │   ├── dependencies/
│   │   ├── applicability/
│   │   └── licenses/
│   │
│   ├── model_import/
│   │   ├── discovery/
│   │   ├── inspection/
│   │   ├── importers/
│   │   ├── source_mapping/
│   │   ├── dependency_analysis/
│   │   └── sandbox/
│   │
│   ├── composition/
│   │   ├── ports/
│   │   ├── compatibility/
│   │   ├── coupling/
│   │   ├── scale_mapping/
│   │   ├── responsibility/
│   │   └── numerical_validation/
│   │
│   ├── simulation/
│   │   ├── runtime/
│   │   ├── scheduler/
│   │   ├── state/
│   │   ├── events/
│   │   ├── checkpoints/
│   │   ├── ensemble/
│   │   └── execution/
│   │
│   ├── interventions/
│   │   ├── genetic/
│   │   ├── molecular/
│   │   ├── cellular/
│   │   ├── tissue/
│   │   ├── mechanical/
│   │   ├── chemical/
│   │   └── environmental/
│   │
│   ├── data_fabric/
│   │   ├── connectors/
│   │   ├── ingestion/
│   │   ├── normalization/
│   │   ├── quality/
│   │   ├── registry/
│   │   ├── lineage/
│   │   └── observation/
│   │
│   ├── model_learning/
│   │   ├── feasibility/
│   │   ├── calibration/
│   │   ├── parameter_estimation/
│   │   ├── structure_refinement/
│   │   ├── model_construction/
│   │   ├── surrogate_models/
│   │   └── optimization/
│   │
│   ├── validation/
│   │   ├── verification/
│   │   ├── scientific_validation/
│   │   ├── benchmarks/
│   │   ├── uncertainty/
│   │   ├── regression/
│   │   └── independent_evaluation/
│   │
│   └── governance/
│       ├── provenance/
│       ├── audit/
│       ├── permissions/
│       ├── approvals/
│       └── change_control/
│
├── biological_libraries/
│   ├── molecular/
│   ├── gene_regulation/
│   ├── cellular/
│   ├── signaling/
│   ├── tissue/
│   ├── mechanics/
│   ├── neural/
│   ├── metabolism/
│   └── physiology/
│
├── model_packages/
│   ├── molecular/
│   ├── cellular/
│   ├── tissue/
│   ├── organ/
│   ├── organ_system/
│   ├── organism/
│   └── composite/
│
├── engine_adapters/
│   ├── compucell3d/
│   ├── physicell/
│   ├── vcell/
│   ├── copasi/
│   ├── chaste/
│   ├── neuron/
│   ├── pk_sim/
│   └── ml_runtime/
│
├── dataset_connectors/
│   ├── ncbi_geo/
│   ├── cellxgene/
│   ├── hubmap/
│   ├── huggingface/
│   └── local/
│
├── experiments/
│   ├── scenarios/
│   ├── baselines/
│   ├── benchmarks/
│   └── manifests/
│
├── integrations/
│   └── scientific_instrument/
│
├── schemas/
├── tests/
├── docs/
├── infrastructure/
└── deployment/
```

این درخت ساختار هدف است. لازم نیست از روز اول برای تک‌تک فولدرها کد بنویسیم.

## ۴۶. تفاوت Library و Model Package

این تفاوت باید از ابتدا مشخص باشد.

**Biological Library:** قابلیت مشترک و قابل‌استفاده مجدد.

مثلاً یک مکانیزم انتشار ماده یا چرخه سلولی.

**Model Package:** یک مدل اجرایی مشخص برای یک زمینه زیستی.

مثلاً مدل رشد اپی‌تلیوم قرنیه انسان.

**Composite Model:** ترکیب نسخه‌های مشخصی از چند مدل.

مثلاً:

```text
Composite Model: Human Eye v0.1
│
├── Cornea Model v1.2
├── Optical Model v0.4
├── Retina Model v0.3
└── Coupling Models
    ├── Cornea-to-Optics
    └── Optics-to-Retina
```

اگر مدل Retina را تغییر دهیم، باید نسخه جدید Composite Model ساخته شود.

نسخه‌های قدیمی باید قابل بازتولید باقی بمانند.

---

# بخش هفدهم: API و ارتباط با سیستم‌های خارجی

## ۴۷. API عمومی

UBSP باید از طریق API و SDK قابل استفاده باشد.

دسته‌های اصلی:

```text
BIOLOGY

GET    /entities
GET    /entities/{id}
GET    /entities/{id}/relationships


MODELS

GET    /models
GET    /models/{id}
GET    /models/{id}/versions
GET    /models/{id}/capabilities
GET    /models/{id}/validity

POST   /models/import
POST   /models/compose
POST   /models/validate


DATASETS

POST   /datasets/register
POST   /datasets/inspect
GET    /datasets/{id}
GET    /datasets/{id}/lineage


SCENARIOS

POST   /scenarios
GET    /scenarios/{id}
POST   /scenarios/{id}/branch


SIMULATIONS

POST   /simulations
GET    /simulations/{id}
POST   /simulations/{id}/cancel
GET    /simulations/{id}/results


MODEL LEARNING

POST   /models/{id}/calibrate
POST   /models/{id}/improvements
POST   /models/construct


VALIDATION

POST   /validation/runs
GET    /validation/reports/{id}
GET    /models/{id}/benchmarks


PROVENANCE

GET    /runs/{id}/provenance
GET    /models/{id}/dependencies
GET    /datasets/{id}/impacted-models
```

این‌ها Endpointهای پیشنهادی هستند.

تمام عملیات باید با Authentication، Authorization، محدودیت منابع و نسخه‌بندی Schema همراه باشند.

## ۴۸. Scientific Instrument Adapter

برای اتصال به پروژه Autonomous AI Research Laboratory، از یک Adapter استفاده می‌کنیم.

```text
Autonomous Research Lab
         |
         v
Scientific Instrument Adapter
         |
         v
UBSP Experiment API
         |
         v
Simulation Planning
         |
         v
Execution & Validation
         |
         v
Results / Artifacts / Uncertainty
         |
         v
Autonomous Research Lab
```

Adapter باید عملیات عمومی Instrument را پشتیبانی کند:

- معرفی قابلیت‌ها
- اعتبارسنجی درخواست
- تخمین هزینه
- ارسال Job
- مشاهده Status
- جمع‌آوری نتایج
- لغو Job

این همان قرارداد عمومی Instrument Blueprint ارسالی است. Pasted text

اما جزئیات علمی Simulation داخل UBSP باقی می‌مانند.

مثلاً Research Lab لازم نیست بداند برای اجرای یک مدل قرنیه، CompuCell3D در پشت صحنه استفاده شده یا یک موتور دیگر.

فقط باید بداند:

- چه مدلی اجرا شده؟
- تحت چه شرایطی؟
- نتیجه چیست؟
- عدم‌قطعیت چقدر است؟
- اعتبار علمی آن برای سؤال فعلی چیست؟
- داده‌ها و Artifactهای خروجی کجا هستند؟

---

# بخش هجدهم: معماری زیرساخت و Technology Stack

## ۴۹. انتخاب فناوری‌ها

با توجه به مقیاس اولیه و نیاز به توسعه علمی، این Stack را پیشنهاد می‌کنم.

| Component | Technology |
|---|---|
| Backend | Python + FastAPI |
| Contracts | Pydantic + JSON Schema |
| Scientific Units | Pint |
| Numerical Computing | NumPy + SciPy |
| Machine Learning | PyTorch |
| Statistical Modeling | statsmodels + PyMC |
| Optimization | BoTorch |
| Primary Database | PostgreSQL |
| Graph | ابتدا PostgreSQL، سپس در صورت نیاز Graph DB |
| Object Storage | S3-compatible Storage |
| Experiment Tracking | MLflow |
| Dataset Processing | AnnData، SpatialData، Xarray |
| Large Imaging Data | Zarr / OME-Zarr |
| Job Queue | Queue + Python Workers |
| Containers | Docker |
| HPC | SLURM Adapter |
| API Documentation | OpenAPI |
| Web Interface | Next.js |
| Graph Visualization | React Flow یا ابزار مشابه |
| Scientific Visualization | Plotly و ابزارهای تخصصی |
| Monitoring | OpenTelemetry |
| CI/CD | GitHub Actions |

### چرا Python؟

چون بسیاری از ابزارهای Computational Biology و Machine Learning رابط Python دارند.

اما این به معنی اجرای تمام محاسبات با Python خالص نیست.

موتورهای C++، Rust یا سایر زبان‌ها می‌توانند از طریق Adapter در Runtime استفاده شوند.

### چرا فعلاً Microservice نه؟

چون مرزهای علمی مدل‌ها هنوز در حال شکل‌گیری هستند.

اگر از ابتدا سیستم را به Microserviceهای متعدد تقسیم کنیم، ممکن است وقت زیادی صرف زیرساخت ارتباطی کنیم، درحالی‌که هنوز ثابت نکرده‌ایم دو مدل زیستی را درست به هم متصل می‌کنیم.

**Modular Monolith برای شروع انتخاب مناسب‌تری است.**

موتورهای محاسباتی سنگین از ابتدا می‌توانند در Process یا Containerهای جدا اجرا شوند.

این جداسازی با Monolith بودن بخش مدیریتی تناقض ندارد.

---

# بخش نوزدهم: رابط کاربری

## ۵۰. شش Workspace اصلی

| Workspace | کاربرد |
|---|---|
| Biological Explorer | مشاهده ساختارها و ارتباطات زیستی |
| Model Studio | مدیریت و اتصال مدل‌ها |
| Dataset Studio | بررسی و استانداردسازی داده‌ها |
| Experiment Studio | تعریف آزمایش و مداخله |
| Simulation Viewer | مشاهده تغییرات سیستم در زمان |
| Validation Center | بررسی دقت و اعتبار مدل‌ها |

### Biological Explorer

مثلاً کاربر وارد بخش Human می‌شود.

بعد Eye را انتخاب می‌کند.

سپس Retina.

در نهایت Retinal Ganglion Cell.

سیستم باید بتواند اطلاعات مرتبط را نمایش دهد:

- محل سلول
- ویژگی‌های شناخته‌شده
- مسیرهای مولکولی مهم
- مدل‌های موجود
- دیتاست‌های مرتبط
- Interactionهای شناخته‌شده
- قسمت‌های فاقد مدل
- اعتبار مدل‌های موجود

### Model Studio

کاربر مدل قرنیه را انتخاب می‌کند.

یک مدل مولکولی جدید را کنار آن قرار می‌دهد.

سیستم بررسی می‌کند آیا اتصال ممکن است یا خیر.

اگر Input و Output سازگار نباشند، توضیح می‌دهد کدام قسمت به Adapter یا Coupling Model نیاز دارد.

### Experiment Studio

کاربر وضعیت اولیه را انتخاب می‌کند.

یک مداخله تعریف می‌کند.

بعد می‌تواند چند Scenario بسازد و آن‌ها را با هم مقایسه کند.

### Simulation Viewer

باید دو نوع نمایش داشته باشیم:

**Structural View:** ساختار سلول، بافت یا اندام.

**Dynamic View:** تغییرات در طول زمان.

اما Viewer نباید وانمود کند جزئیاتی را مشاهده می‌کنیم که مدل محاسبه نکرده است.

مثلاً اگر مدل فقط تعداد سلول‌ها را محاسبه می‌کند، نباید یک انیمیشن سه‌بعدی را به‌عنوان بازسازی واقعی حرکت تک‌تک سلول‌ها معرفی کند.

---

# بخش بیستم: نقشه توسعه پیشنهادی

## ۵۱. از کجا شروع کنیم؟

من پروژه را به هشت فاز تقسیم می‌کنم.

این مراحل ترتیب وابستگی‌های فنی و علمی را نشان می‌دهند. همه قابلیت‌ها لازم نیست به‌صورت کاملاً ترتیبی توسعه پیدا کنند.

| فاز | هدف | خروجی قابل بررسی |
|---|---|---|
| **P0** | Scientific Contracts و Core Architecture | قراردادهای مدل، داده، اجرا و اعتبار |
| **P1** | اجرای vCornea | بازتولید نتایج مدل اصلی |
| **P2** | Data & Validation | اولین Benchmark مستقل |
| **P3** | Model Learning | بهبود کنترل‌شده یک مدل |
| **P4** | Model Composition | ترکیب معتبر دو مدل |
| **P5** | Genetic & Molecular Integration | اولین سناریوی چندمقیاسی ژن تا بافت |
| **P6** | Integrated Organ | مدل ترکیبی یک اندام |
| **P7** | Multi-Organ & Adaptive Simulation | اولین سناریوی معتبر چنداندامی |

## فاز صفر: Foundation

**هدف:** ساخت قراردادها و زیرساخت اولیه.

قابلیت‌ها:

- Model Registry
- Scientific Model Contract
- Dataset Registry
- Simulation Run Manifest
- Engine Adapter Protocol
- Provenance
- Basic Validation
- Scenario Definition

معیار تکمیل:

سیستم بتواند دو مدل آزمایشی با ساختار متفاوت را بدون تغییر در Core ثبت و اجرا کند.

---

## فاز اول: vCornea Integration

**هدف:** اولین شبیه‌سازی زیستی واقعی.

فعالیت‌ها:

- وارد کردن مدل اصلی
- تثبیت نسخه CompuCell3D
- بازتولید سناریوهای منتشرشده
- استخراج خروجی‌های استاندارد
- ثبت پارامترها و نتایج
- اجرای چند Run مستقل

معیار تکمیل:

بتوانیم سناریوی مشخصی را با نسخه ثبت‌شده مدل اجرا کنیم و نتایج آن را در محدوده تلرانس تعریف‌شده با اجرای مرجع مقایسه کنیم.

---

## فاز دوم: Scientific Validation

**هدف:** اثبات اینکه سیستم فقط اجرا نمی‌کند، بلکه کیفیت مدل را هم می‌سنجد.

فعالیت‌ها:

- انتخاب Endpoint مشخص
- شناسایی داده مرجع مناسب
- بررسی کیفیت داده
- ساخت Observation Operator
- محاسبه خطای مدل
- Sensitivity Analysis
- گزارش عدم‌قطعیت

معیار تکمیل:

یک Validation Report قابل بازتولید برای یک کاربرد مشخص داشته باشیم.

اگر داده مستقل مناسب پیدا نشد، باید صریحاً همان محدودیت را گزارش کنیم.

---

## فاز سوم: Model Improvement

**هدف:** اولین بهبود قابل دفاع در یک شبیه‌ساز زیستی.

فعالیت‌ها:

- انتخاب یک ضعف مشخص
- بررسی شواهد مربوط
- ساخت Candidate Model
- اجرای Benchmarkهای قبلی
- مقایسه با نسخه مرجع
- ارزیابی مستقل

معیار تکمیل:

یا یک بهبود معتبر اثبات شود، یا نتیجه نشان دهد که تغییر پیشنهادی مفید نبوده است.

**حتی نتیجه منفی هم می‌تواند موفقیت زیرساخت باشد**، اگر سیستم توانسته باشد آزمایش را درست طراحی، اجرا و ارزیابی کند.

---

## فاز چهارم: Model Composition

**هدف:** ثابت کنیم می‌توانیم دو مدل مستقل را درست ترکیب کنیم.

پیشنهاد من اتصال یک مدل مولکولی به vCornea است.

اما ابتدا باید یک ارتباط علمی دارای داده کافی انتخاب کنیم.

معیار تکمیل:

- مدل‌ها مستقل اجرا شوند.
- اتصال آن‌ها از نظر واحد و مقیاس سازگار باشد.
- ترکیب آن‌ها پایدار باشد.
- خروجی ترکیبی با داده مرتبط بررسی شود.
- بتوانیم اثر Coupling را از اثر مدل‌های مستقل تفکیک کنیم.

این فاز از نظر معماری شاید مهم‌ترین Milestone پروژه باشد.

---

## فاز پنجم: Genetics to Tissue

**هدف:** ایجاد اولین مسیر چندمقیاسی از مداخله ژنتیکی تا پاسخ بافت.

```text
Genetic Perturbation
        |
        v
Gene Response Model
        |
        v
Cellular Model
        |
        v
Tissue Simulation
```

معیار تکمیل:

یک تغییر مشخص را در یک زمینه زیستی دارای داده کافی بررسی کنیم و نشان دهیم که خروجی هر مرحله چه میزان پشتوانه تجربی دارد.

لازم نیست اولین کاربرد حتماً یک درمان جدید باشد.

---

## فاز ششم: Integrated Eye

**هدف:** ساخت اولین مدل ترکیبی چشم برای یک سؤال علمی محدود.

مثلاً:

«تغییر مشخصی در سطح قرنیه چه اثری بر کیفیت نوری تصویر رسیده به شبکیه دارد؟»

در این مرحله نباید مدعی ساخت Digital Twin کامل چشم باشیم.

معیار تکمیل:

مدل ترکیبی بتواند برای Endpoint انتخاب‌شده خروجی معتبرتری از یک مدل ساده مرجع تولید کند، یا محدودیت‌های آن را به‌صورت کمی مشخص کند.

---

## فاز هفتم: Multi-Organ Simulation

**هدف:** بررسی یک تعامل واقعی میان دو اندام.

مثلاً:

```text
Liver Metabolism
       |
       v
Circulating Concentration
       |
       v
Eye Tissue Exposure
       |
       v
Cellular Response
```

معیار تکمیل:

مدل ترکیبی بتواند یک رابطه بین‌اندامی مشخص را در شرایط دارای داده مرجع شبیه‌سازی و اعتبارسنجی کند.

این Milestone اولین قدم جدی به سمت Whole-Body Modeling است.

---

# بخش بیست‌ویکم: تصمیم‌های نهایی معماری

## ۵۲. Architecture Decision Records

| موضوع | تصمیم |
|---|---|
| Product | Universal Biological Simulation Platform |
| Scope | General-Purpose Biological Modeling & Simulation |
| First Domain | Corneal Epithelium |
| Initial Scientific Model | vCornea |
| Architecture | Modular Monolith |
| Repository | مستقل از Research Lab |
| External Integration | Scientific Instrument Adapter |
| Biological Structure | Hierarchy + Interaction Graph |
| Model Management | Versioned Scientific Model Packages |
| Component Reuse | Composition-Based Biological Libraries |
| Model Sources | Existing Models + Custom Models + Data-Driven Models |
| Import Strategy | Reuse / Adapt / Compose / Build |
| Model Discovery | Capability-Based |
| Model Execution | Multiple Engines |
| Cross-Model Execution | Coupled Multiscale Runtime |
| Input & Output | Typed Scientific Ports |
| State | Independent Simulation State Store |
| Geometry | Spatial Mapping & Reference Systems |
| Time | Multi-Rate Scheduling |
| Interventions | General Intervention Framework |
| Genetics | Genetic & Molecular Perturbation Support |
| Regeneration | Cellular, Tissue & Functional Models |
| Data | Multimodal Scientific Data Fabric |
| Model Learning | Calibration + ML + Model Refinement |
| Model Evolution | Versioned Champion / Challenger |
| Trust | Context-Specific Scientific Validation |
| Unknown Knowledge | Explicit Missing Knowledge Registry |
| Cross-Species | Evidence-Based Mapping |
| Model Dependencies | Versioned Dependency Graph |
| Reproducibility | Immutable Run Manifests |
| Provenance | PROV-O / RO-Crate-compatible |
| Security | Isolated Model Execution |
| Governance | Independent Validation & Controlled Changes |
| Future Direction | Integrated Organism Simulation |

---

# بخش بیست‌ودوم: چه چیزهایی را نباید از ابتدا بسازیم؟

به نظرم این قسمت هم مهم است، چون یک پروژه با چنین چشم‌انداز بزرگی به‌سادگی می‌تواند از کنترل خارج شود.

| چیزی که فعلاً نمی‌سازیم | دلیل |
|---|---|
| مدل اتمی تمام بدن | هزینه محاسباتی و کمبود دانش |
| مدل کامل تمام سلول‌های انسان | داده و مدل معتبر کافی نداریم |
| یک Foundation Model که جای تمام موتورهای فیزیکی را بگیرد | قابلیت تعمیم علمی اثبات نشده |
| موتور اختصاصی جدید برای تمام انواع Simulation | ابزارهای تخصصی موجودند |
| ده‌ها Microservice | پیچیدگی زودهنگام |
| یک Graph Database برای تمام داده‌های زمانی و مکانی | مناسب نبودن برای تمام انواع داده |
| سیستم خودکار تأیید علمی مدل‌های خودش | تعارض منافع در ارزیابی |
| RL عمومی برای تمام تحقیقات | نبود Reward و محیط معتبر عمومی |
| Digital Twin کامل انسان از روز اول | مسئله علمی حل‌نشده |
| شبیه‌سازی تمام بیماری‌ها با یک مدل واحد | تفاوت شدید مکانیزم‌های زیستی |

هدف این نیست که هیچ‌وقت به این حوزه‌ها نزدیک نشویم.

هدف این است که قبل از اثبات زیرساخت اصلی، منابع را صرف حل همه مسائل زیست‌شناسی نکنیم.

---

# بخش بیست‌وسوم: قابلیت گسترش به موجودات دیگر

## ۵۳. معماری نباید Human-Specific باشد

درست است که اولین کاربرد ما چشم انسان است، اما Biological Core نباید انسان را تنها گونه موجود فرض کند.

برای هر مدل باید Species مشخص باشد.

بعداً می‌توانیم از مدل‌های:

- انسان
- موش
- سایر پستانداران
- موجودات ساده‌تر
- باکتری‌ها
- گیاهان

پشتیبانی کنیم.

اما **Species یک Configuration ساده نیست که فقط با تغییر نام عوض شود.**

مدل سلول انسان را نمی‌توان لزوماً بدون تغییر برای سلول گیاه استفاده کرد.

ساختار، متابولیسم و ارتباطات زیستی می‌توانند متفاوت باشند.

پیشنهاد می‌کنم **Species-Aware Model Registry** داشته باشیم.

این Registry مشخص می‌کند هر مدل برای کدام گونه توسعه یافته و انتقال آن به گونه دیگر چه شواهدی نیاز دارد.

## ۵۴. گسترش از Organism به Population

در آینده ممکن است بخواهیم رفتار چند موجود زنده را نیز بررسی کنیم.

مثلاً:

- انتقال یک ماده میان موجود و محیط
- رفتار یک جمعیت سلولی
- ارتباط موجود زنده با میکروبیوم
- تعامل چند موجود در یک محیط

بنابراین بهتر است مدل داده در آینده بتواند این ساختار را هم پشتیبانی کند:

```text
Molecule
   |
   v
Cell
   |
   v
Tissue
   |
   v
Organ
   |
   v
Organ System
   |
   v
Organism
   |
   v
Population
   |
   v
Environment / Ecosystem
```

این ساختار نمای مقیاس‌های تحلیل است، نه یک زنجیره قطعی که هر سطح از نظر فیزیکی فقط زیرمجموعه سطح بعدی باشد.

در نسخه اولیه، Population و Ecosystem فقط در Schema و مسیر توسعه در نظر گرفته می‌شوند.

نیازی نیست موتورهای آن‌ها را از ابتدا بسازیم.

---

# بخش بیست‌وچهارم: معیار موفقیت کل پروژه

## ۵۵. چه زمانی می‌توانیم بگوییم UBSP واقعاً موفق شده است؟

به نظرم موفقیت را نباید با تعداد مدل‌ها، حجم داده‌ها یا پیچیدگی UI بسنجیم.

باید آن را بر اساس قابلیت‌های اثبات‌شده اندازه‌گیری کنیم.

### Milestone A: Reproducible Simulation

یک مدل علمی منتشرشده را وارد کنیم و نتایج مرجع آن را در محدوده قابل‌قبول بازتولید کنیم.

### Milestone B: Scientific Validation

یک مدل را با داده مستقل واقعی ارزیابی کنیم و خطاها و محدودیت‌های آن را گزارش دهیم.

### Milestone C: Validated Model Improvement

نشان دهیم نسخه جدید یک مدل در کاربرد مشخص، بهبود قابل دفاعی نسبت به نسخه قبلی دارد.

### Milestone D: Validated Model Composition

دو مدل مستقل را طوری ترکیب کنیم که نتیجه ترکیبی نه‌تنها از نظر فنی اجرا شود، بلکه در برابر داده و قیود علمی نیز ارزیابی شود.

### Milestone E: Cross-Scale Prediction

یک مداخله را در مقیاس پایین‌تر تعریف کنیم و اثر آن را در مقیاس بالاتر بررسی کنیم.

مثلاً از Gene Expression به Cell Behavior و Tissue Response برسیم.

### Milestone F: Cross-Organ Simulation

تعامل میان دو اندام را برای یک پدیده مشخص شبیه‌سازی و با شواهد واقعی ارزیابی کنیم.

### Milestone G: Scientific Model Evolution

سیستم بتواند به‌صورت کنترل‌شده:

- یک ضعف مدل را شناسایی کند.
- یک اصلاح پیشنهاد دهد.
- نسخه جدید بسازد.
- آن را آزمایش کند.
- نتیجه را مستقل ارزیابی کند.
- نسخه مناسب را ارتقا دهد یا رد کند.

**این آخرین Milestone به‌نظرم یکی از ارزشمندترین قابلیت‌های بلندمدت پلتفرم است.**

چون باعث می‌شود UBSP فقط محل اجرای مدل‌ها نباشد، بلکه زیرساخت توسعه و ارزیابی مداوم مدل‌های زیستی شود.

---

# جمع‌بندی نهایی معماری

پس از بررسی طراحی‌های قبلی، سناریوهای مختلف و پروژه‌های موجود، **معماری نهایی پیشنهادی من ترکیب پنج قابلیت بنیادی است**:

### ۱. Biological Representation

نمایش استاندارد اجزا، مکانیزم‌ها، ساختارها، وضعیت‌ها و تعاملات سیستم‌های زنده.

### ۲. Model Ecosystem

وارد کردن، ساخت، نگهداری، نسخه‌بندی و استفاده مجدد از مدل‌های زیستی.

### ۳. Multiscale Simulation

ترکیب و اجرای مدل‌ها در مقیاس‌های مولکولی، سلولی، بافتی، اندامی و سیستمی.

### ۴. Scientific Data & Validation

دریافت داده‌های چندوجهی، ارزیابی کیفیت آن‌ها و مقایسه خروجی مدل‌ها با واقعیت.

### ۵. Model Learning & Evolution

بهبود کنترل‌شده مدل‌ها بر اساس داده و شواهد جدید، بدون از دست دادن اعتبار، تاریخچه یا قابلیت بازتولید.

این پنج بخش به‌صورت مستقل طراحی می‌شوند، اما با قراردادهای علمی مشترک همکاری می‌کنند.

## مهم‌ترین اصل معماری

**نباید یک مدل بسیار بزرگ و غیرقابل‌تغییر از موجود زنده بسازیم.**

باید اکوسیستمی از مدل‌های تخصصی، قابل‌تعویض، قابل‌ترکیب و نسخه‌دار داشته باشیم.

در این اکوسیستم می‌توانیم:

- یک مولکول را مستقل بررسی کنیم.
- رفتار یک سلول را شبیه‌سازی کنیم.
- تعامل جمعیتی از سلول‌ها را مشاهده کنیم.
- یک بافت را مدل کنیم.
- چند بافت را به یک اندام متصل کنیم.
- چند اندام را در یک سیستم فیزیولوژیکی ترکیب کنیم.
- مداخله‌ای را در یک بخش اعمال کنیم و اثرهای قابل مدل‌سازی آن را در بخش‌های دیگر بررسی کنیم.
- مدل‌های ناقص را شناسایی کنیم.
- با داده‌های تازه، مدل‌های موجود را اصلاح یا مدل‌های جدید ایجاد کنیم.

اما تمام این قابلیت‌ها باید یک شرط ثابت داشته باشند:

**هیچ پیش‌بینی‌ای نباید معتبرتر از شواهدی که مدل و ارتباطات آن را پشتیبانی می‌کنند معرفی شود.**

### اولین خروجی واقعی که باید هدف بگیریم

من اولین موفقیت پروژه را این‌گونه تعریف می‌کنم:

> «UBSP بتواند vCornea را از منبع اصلی وارد کند، آن را بدون تغییر علمی اجرا کند، نتایج را با داده مستقل مقایسه کند، یک ضعف قابل‌اندازه‌گیری را شناسایی کند، یک نسخه اصلاح‌شده بسازد و نشان دهد آن نسخه در شرایط مشخص واقعاً بهتر یا بدتر از مدل اصلی است.»

بعد از آن، **اتصال یک مدل مولکولی به vCornea** مهم‌ترین آزمون معماری خواهد بود.

اگر این دو دستاورد را درست انجام دهیم، پایه علمی و فنی لازم برای توسعه مدل‌های پیچیده‌تر چشم، ژنتیک، پزشکی بازساختی و در نهایت مدل‌سازی چنداندامی را خواهیم داشت.

**ارزش واقعی UBSP در شبیه‌سازی همه‌چیز از روز اول نیست. در ساخت زیرساختی است که بتواند دانش زیستی معتبر را به مدل اجرایی تبدیل کند، مدل‌ها را به‌درستی ترکیب کند و با شواهد جدید، ظرفیت پیش‌بینی خود را گسترش دهد.**