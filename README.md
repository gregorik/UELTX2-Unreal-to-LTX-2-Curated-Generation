# UELTX2: Curated Generation (LTX-2 Bridge for UE5)
![image](https://img.shields.io/badge/-Unreal%20Engine-313131?style=for-the-badge&logo=unreal-engine&logoColor=blue) ![image](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=blue) ![image](https://img.shields.io/badge/Python-FFD43B?style=for-the-badge&logo=python&logoColor=blue) ![image](https://img.shields.io/badge/json-5E5C5C?style=for-the-badge&logo=json&logoColor=white) ![image](https://img.shields.io/badge/MIT-green?style=for-the-badge) ![alt text](https://img.shields.io/github/stars/gregorik/InstantOrganicCaves) ![alt text](https://img.shields.io/badge/Support-Patreon-red) [![YouTube](https://img.shields.io/badge/YouTube-Subscribe-red?style=flat&logo=youtube)](https://www.youtube.com/@agregori) [![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/C0C616ULD4)

**UELTX2** is a native Unreal Engine 5 plugin that integrates **Lightricks LTX-2**, a state-of-the-art generative video model. It allows developers to generate 4K cinematic video assets, dynamic textures, and animatics directly inside the Unreal Editor.

This plugin operates as a "Bridge." It connects Unreal Engine (Frontend) to a local **ComfyUI** instance (Backend) handling the heavy AI inference.

---

## 📋 Prerequisites

### Hardware Requirements
LTX-2 is a heavy DiT (Diffusion Transformer) model.
*   **GPU:** NVIDIA RTX 3090 / 4090 (24GB VRAM) recommended for full precision.
    *   *Note: RTX 3060/4070 (12GB+) can run the model if using GGUF Quantization.*
*   **RAM:** 32GB+ System RAM.
*   **Storage:** SSD with at least 20GB free space for models.

### Software Requirements
*   **Unreal Engine:** 5.5+.
*   **OS:** Windows 10/11.
*   **ComfyUI:** Latest version (Installed locally).

---

## 🛠️ Phase 1: Setting up ComfyUI (The Backend)

You must have ComfyUI running locally for this plugin to work.

### 1. Install ComfyUI
If you haven't installed it yet, usage of the standalone portable version is recommended for Windows users to avoid Python dependency hell.
1.  Download the **ComfyUI_windows_portable_nvidia.7z** from the [Official GitHub](https://github.com/comfyanonymous/ComfyUI/releases).
2.  Extract it to a short path, e.g., `C:\ComfyUI`.

### 2. Install The Node Manager (Crucial)
1.  Go to `C:\ComfyUI\ComfyUI\custom_nodes`.
2.  Open a terminal (CMD) in this folder.
3.  Run: `git clone https://github.com/ltdrdata/ComfyUI-Manager.git`
4.  Restart ComfyUI.

### 3. Install Required Custom Nodes
Unreal needs specific nodes to be present in ComfyUI to handle the JSON payloads.
1.  Open ComfyUI in your browser (`http://127.0.0.1:8188`).
2.  Click **"Manager"** in the floating menu.
3.  Click **"Install Custom Nodes"**.
4.  Search for and install the following:
    *   **ComfyUI-VideoHelperSuite** (Required for saving MP4s).
    *   **ComfyUI-GGUF** (Highly recommended for LTX-2 memory optimization).
    *   **ComfyUI-Inspire-Pack** (Optional, keeps workflows clean).

---

## 📥 Phase 2: Downloading the Models

You need the specific LTX-2 weights.

### Option A: High-End GPUs (24GB+ VRAM)
1.  Download standard weights from [HuggingFace Lightricks/LTX-2](https://huggingface.co/Lightricks/LTX-2).
2.  Place `ltx-video-2b-v0.9.safetensors` in:
    `C:\ComfyUI\ComfyUI\models\checkpoints\`

### Option B: Consumer GPUs (8GB - 16GB VRAM) - **Recommended**
1.  Download **GGUF** quantized weights (e.g., Q8_0 or Q5_k_m) from [City96 or Unsloth](https://huggingface.co/City96/LTX-Video-2b-v0.9-GGUF).
2.  Place the `.gguf` file in:
    `C:\ComfyUI\ComfyUI\models\checkpoints\`
3.  *Note: You may need to edit the `LTX2_T2V.json` template in the plugin folder to point to this new filename if it differs.*

---

## 🔌 Phase 3: Plugin Installation

1.  Copy the `UELTX2` folder into your project's `Plugins/` directory.
    *   Path: `YourProject/Plugins/UELTX2/`
2.  Right-click your `.uproject` file and select **Generate Visual Studio Project Files**.
3.  Open the `.sln` solution.
4.  **Build** the project in "Development Editor" configuration.
5.  Open Unreal Engine.
6.  Go to **Edit > Plugins**, search for **UELTX2**, and ensure it is enabled.

---

## 🚀 Phase 4: Usage Workflow

### 1. Start the Server
Before working in Unreal, double-click `run_nvidia_gpu.bat` in your ComfyUI folder. Ensure you see:
> `Starting server`
> `To see the GUI go to: http://127.0.0.1:8188`

### 2. Configure Unreal
1.  In UE5, go to **Project Settings > Game > LTX-2 Generation**.
2.  Ensure **Comfy URL** matches your server (Default: `http://127.0.0.1:8188`).
3.  Ensure **Auto Import** is checked.

### 3. Text-to-Video (T2V)
1.  In the Level Editor, click the **"LTX-2"** button on the main toolbar.
2.  A generic generation panel will open (if configured) or the system works via context actions.
    *(See `Content/UI/EUW_UELTX2_Panel` for the Frontend implementation).*

### 4. Image-to-Video (I2V) - *The Moving Texture Workflow*
1.  Import a texture or screenshot into the **Content Browser**.
2.  **Right-click** the Texture Asset.
3.  Navigate to **Generative AI > Animate with LTX-2 (I2V)**.
4.  Check the "Output Log" for status.
    *   *LogUELTX2: Sending prompt to ComfyUI...*
5.  Wait approx. 2-5 minutes (depending on GPU).
6.  The video will appear in `/Game/UELTX2_Generations/` as a MediaSource.

---

## ⚠️ Troubleshooting

**Q: "Connection Refused" in Output Log?**
A: ComfyUI is not running, or the URL in Project Settings is wrong. Make sure port 8188 is open.

**Q: ComfyUI shows red nodes when receiving the JSON?**
A: You are missing nodes.
1.  In ComfyUI, drag and drop `Plugins/UELTX2/Content/Templates/LTX2_T2V.json` directly onto the ComfyUI browser window.
2.  Use **Manager > Install Missing Custom Nodes** to automatically find what you lack.

**Q: Unreal freezes during generation?**
A: It shouldn't. The request is asynchronous. However, if your GPU VRAM is maxed out by ComfyUI, Unreal might stutter.
*   *Fix:* Enable "Limit Editor Frame Rate" in UE5 when in background.
*   *Fix:* Use GGUF LTX-2 weights to save VRAM.

---

## 📄 License
This plugin logic is MIT. The LTX-2 Model weights are subject to Lightricks' Open Access License (Non-Commercial/Research usually, check HuggingFace for updates).
```
