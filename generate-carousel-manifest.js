#!/usr/bin/env node

/**
 * Carousel Manifest Generator
 * Automatically scans Pictures/carousel folder and generates manifest.json
 * Run: node generate-carousel-manifest.js
 */

const fs = require('fs');
const path = require('path');

const CAROUSEL_DIR = path.join(__dirname, 'Pictures', 'carousel');
const MANIFEST_FILE = path.join(CAROUSEL_DIR, 'manifest.json');

// Supported media types
const SUPPORTED_TYPES = {
  // Images
  '.jpeg': 'image',
  '.jpg': 'image',
  '.png': 'image',
  '.gif': 'image',
  '.webp': 'image',
  // Videos
  '.mp4': 'video',
  '.webm': 'video',
  '.mov': 'video',
  '.avi': 'video'
};

function generateManifest() {
  try {
    // Check if carousel directory exists
    if (!fs.existsSync(CAROUSEL_DIR)) {
      console.error(`❌ Carousel directory not found: ${CAROUSEL_DIR}`);
      process.exit(1);
    }

    // Read all files in the carousel directory
    const files = fs.readdirSync(CAROUSEL_DIR);

    // Filter and sort media files
    const mediaFiles = files
      .filter(file => {
        const ext = path.extname(file).toLowerCase();
        return SUPPORTED_TYPES[ext];
      })
      .sort((a, b) => {
        // Sort alphabetically/numerically
        return a.localeCompare(b, undefined, { numeric: true });
      });

    if (mediaFiles.length === 0) {
      console.warn('⚠️  No media files found in carousel directory');
    }

    // Create manifest items
    const items = mediaFiles.map(file => ({
      type: SUPPORTED_TYPES[path.extname(file).toLowerCase()],
      src: `Pictures/carousel/${file}`
    }));

    // Generate manifest JSON
    const manifest = {
      items: items,
      generated: new Date().toISOString(),
      totalItems: items.length
    };

    // Write manifest file
    fs.writeFileSync(MANIFEST_FILE, JSON.stringify(manifest, null, 2));

    console.log('✅ Carousel manifest generated successfully!');
    console.log(`📁 Directory: ${CAROUSEL_DIR}`);
    console.log(`📊 Total items: ${items.length}`);
    console.log('\n📋 Items:');
    items.forEach((item, i) => {
      console.log(`  ${i + 1}. [${item.type.toUpperCase()}] ${path.basename(item.src)}`);
    });

  } catch (error) {
    console.error('❌ Error generating manifest:', error.message);
    process.exit(1);
  }
}

// Run if executed directly
if (require.main === module) {
  generateManifest();
}

module.exports = generateManifest;
