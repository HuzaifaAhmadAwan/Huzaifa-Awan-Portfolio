# Carousel Setup Guide

## 🎠 Automatic Image Detection

The carousel now automatically detects new images added to the `Pictures/carousel/` folder!

### How It Works

Simply add new images/videos to the `Pictures/carousel/` folder, then run the generator script to update the manifest.

## 🚀 Setup (One-Time)

Make sure you have Node.js installed. If not, [download it here](https://nodejs.org/)

## 📝 Usage

### Option 1: Command Line (Recommended)

1. **Add your images** to `Pictures/carousel/` folder
2. **Run the generator** from the project root:
   ```bash
   node generate-carousel-manifest.js
   ```
3. **Refresh your portfolio** - new images will appear in the carousel!

### Option 2: Automatic (NPM Script)

Add this to your `package.json`:
```json
{
  "scripts": {
    "carousel": "node generate-carousel-manifest.js"
  }
}
```

Then run:
```bash
npm run carousel
```

## 📂 Supported File Types

**Images:**
- `.jpeg`, `.jpg`
- `.png`
- `.gif`
- `.webp`

**Videos:**
- `.mp4`
- `.webm`
- `.mov`
- `.avi`

## 📋 Example

Folder structure:
```
Pictures/
├── carousel/
│   ├── 1.jpeg
│   ├── 2.jpeg
│   ├── 3.jpeg
│   ├── 4.mp4
│   ├── 5.jpeg
│   ├── my-new-image.jpg      ← Add this
│   └── manifest.json
```

Run:
```bash
node generate-carousel-manifest.js
```

Output:
```
✅ Carousel manifest generated successfully!
📁 Directory: c:\...\Pictures\carousel
📊 Total items: 6

📋 Items:
  1. [IMAGE] 1.jpeg
  2. [IMAGE] 2.jpeg
  3. [IMAGE] 3.jpeg
  4. [VIDEO] 4.mp4
  5. [IMAGE] 5.jpeg
  6. [IMAGE] my-new-image.jpg
```

## 🔄 Automatic Sorting

Files are sorted **alphabetically with numeric support**, so:
- `1.jpeg`, `2.jpeg`, `3.jpeg` → displayed in order
- `image_001.jpg`, `image_002.jpg` → displayed in order
- `a.jpg`, `b.jpg`, `c.jpg` → displayed in order

## 💡 Tips

- **File naming**: Use numbers or alphabetical order to control carousel sequence
- **File sizes**: Keep images under 2MB for fast loading
- **Aspect ratio**: Use uniform dimensions (e.g., 400x300px) for best appearance
- **Video thumbnails**: Videos show as black until hovered and clicked

## ✅ Verification

The `manifest.json` file is automatically updated with:
- List of all media files
- File types (image/video)
- Generation timestamp
- Total item count

Check the manifest file to verify it was generated correctly.
