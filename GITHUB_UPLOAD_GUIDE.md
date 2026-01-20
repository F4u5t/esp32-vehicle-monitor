# GitHub Upload Guide

Step-by-step instructions for uploading this project to GitHub.

## Prerequisites

- [ ] Git installed on your system
- [ ] GitHub account created
- [ ] Project tested and working

## Install Git (if needed)

### Windows:
```powershell
# Download from: https://git-scm.com/download/win
# Or use winget:
winget install Git.Git
```

### Verify Installation:
```powershell
git --version
```

---

## Step-by-Step GitHub Upload

### Step 1: Configure Git (First Time Only)

```powershell
# Set your name
git config --global user.name "Your Name"

# Set your email (use your GitHub email)
git config --global user.email "your.email@example.com"

# Verify
git config --list
```

### Step 2: Initialize Local Repository

```powershell
# Navigate to project folder
cd "C:\Users\Matt\Desktop\SuperDashv7"

# Initialize git repository
git init

# Check status
git status
```

You should see:
```
On branch master
Untracked files:
  .gitignore
  LICENSE
  README.md
  docs/
  firmware/
  hardware/
  laptop/
  ...
```

### Step 3: Add Files to Git

```powershell
# Add all files (respecting .gitignore)
git add .

# Verify what will be committed
git status
```

Should show files in green (staged):
```
Changes to be committed:
  new file:   .gitignore
  new file:   LICENSE
  new file:   README.md
  new file:   firmware/sender/...
  ...
```

**Important:** Planning docs (CONSOLIDATION_PLAN.md, etc.) should NOT appear because they're in .gitignore.

### Step 4: Make Initial Commit

```powershell
git commit -m "Initial commit: ESP32 Vehicle Monitor System

- ESP32C6 sender with oil temp/pressure monitoring
- CYD display with GPS speedometer
- Laptop GPS integration via gpsd
- Complete documentation and setup guides
- Hardware schematics and BOM"
```

### Step 5: Create GitHub Repository

1. **Go to:** https://github.com
2. **Log in** to your account
3. **Click the "+" button** (top right) → "New repository"

**Repository Settings:**
```
Repository name: esp32-vehicle-monitor
Description: ESP32 vehicle monitoring system with oil temp/pressure sensors and GPS speedometer
Public or Private: Choose your preference
```

**Important:**
- ❌ Do NOT check "Initialize this repository with a README"
- ❌ Do NOT add .gitignore
- ❌ Do NOT add a license

(We already have all these files locally)

4. **Click "Create repository"**

### Step 6: Connect Local Repo to GitHub

GitHub will show you commands. Use the "…or push an existing repository from the command line" section:

```powershell
# Add GitHub as remote (replace YOUR_USERNAME and YOUR_REPO_NAME)
git remote add origin https://github.com/YOUR_USERNAME/esp32-vehicle-monitor.git

# Rename branch to main (if needed)
git branch -M main

# Push to GitHub
git push -u origin main
```

**Example:**
```powershell
git remote add origin https://github.com/mattsmith/esp32-vehicle-monitor.git
git branch -M main
git push -u origin main
```

### Step 7: Enter GitHub Credentials

You may be prompted for credentials:

**Option A: Personal Access Token (Recommended)**
1. Go to GitHub → Settings → Developer settings → Personal access tokens
2. Generate new token (classic)
3. Select scopes: `repo` (full control of private repositories)
4. Copy the token
5. Use token as password when prompted

**Option B: GitHub CLI**
```powershell
# Install GitHub CLI
winget install GitHub.cli

# Authenticate
gh auth login
```

### Step 8: Verify Upload

1. **Go to your repository:** https://github.com/YOUR_USERNAME/esp32-vehicle-monitor
2. **Check that you see:**
   - README.md displaying as landing page
   - folder structure (firmware/, docs/, hardware/, laptop/)
   - LICENSE file
   - .gitignore file
3. **Verify these are NOT visible:**
   - CONSOLIDATION_PLAN.md
   - DOCUMENTATION_AUDIT.md
   - PROJECT_REVIEW_SUMMARY.md
   - ESP_Oil_Data/ (if you wanted it excluded)
   - ESP_CYD_1/ (if you wanted it excluded)

---

## Customizing GitHub Repository

### Add Repository Topics

On GitHub repository page:
1. Click ⚙️ (Settings icon) next to "About"
2. Add topics:
   - `esp32`
   - `esp-now`
   - `arduino`
   - `vehicle-monitoring`
   - `gps`
   - `temperature-sensor`
   - `pressure-sensor`
   - `iot`

### Enable GitHub Pages (Optional)

Create a simple documentation website:
1. Go to repository Settings → Pages
2. Source: Deploy from branch
3. Branch: main, folder: / (root)
4. Save

Your README will be visible at: `https://YOUR_USERNAME.github.io/esp32-vehicle-monitor`

### Add Repository Description

Click "Edit" under repository name and add:
```
🚗 ESP32-based vehicle monitoring system combining oil temperature/pressure sensors with GPS speedometer display using ESP-NOW wireless protocol
```

### Add Badges to README (Optional)

Add to top of README.md:

```markdown
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![Status](https://img.shields.io/badge/status-active-green.svg)
```

Then commit and push:
```powershell
git add README.md
git commit -m "Added badges to README"
git push
```

---

## Making Updates

After initial upload, when you make changes:

```powershell
# Check what changed
git status

# Add specific file
git add firmware/sender/sender_arduino.ino

# Or add all changes
git add .

# Commit with message
git commit -m "Fixed pressure sensor calibration"

# Push to GitHub
git push
```

### Commit Message Best Practices

**Good commit messages:**
```
✅ "Fixed temperature sensor timeout issue"
✅ "Added battery voltage monitoring feature"
✅ "Updated wiring diagram for ADS1115"
✅ "Improved ESP-NOW retry logic"
```

**Bad commit messages:**
```
❌ "update"
❌ "fix"
❌ "changes"
❌ "asdf"
```

---

## Managing Old Folders

If you want to keep old folders locally but not push them to GitHub, the .gitignore already handles this. But if you want to be explicit:

### Option 1: Keep Old Folders Locally

Already done! They're in .gitignore, so they won't be pushed.

### Option 2: Delete Old Folders

Once you've verified everything works:

```powershell
# Delete old project folders
Remove-Item -Path "ESP_Oil_Data" -Recurse -Force
Remove-Item -Path "ESP_CYD_1" -Recurse -Force

# Optional: Delete planning docs
Remove-Item "CONSOLIDATION_PLAN.md" -Force
Remove-Item "DOCUMENTATION_AUDIT.md" -Force
Remove-Item "PROJECT_REVIEW_SUMMARY.md" -Force
Remove-Item "MAC_ADDRESS_SETUP.md" -Force

# Git won't notice because they were never tracked
git status  # Should show nothing
```

---

## Adding Images

### Step 1: Take Photos

Capture photos of:
- Complete system overview
- Sender unit in engine bay
- CYD display on dashboard
- Sensor installations (close-up)
- Wiring connections

### Step 2: Add to Repository

```powershell
# Copy images to images folder
Copy-Item "path\to\photo.jpg" "images\sender-installed.jpg"

# Add to git
git add images/

# Commit
git commit -m "Added installation photos"

# Push
git push
```

### Step 3: Reference in README

Edit README.md:

```markdown
## Installation Photos

![Sender Unit](images/sender-installed.jpg)
*ESP32C6 sender unit installed in engine bay*

![Dashboard Display](images/dashboard-display.jpg)
*CYD display showing real-time data*
```

Then:
```powershell
git add README.md
git commit -m "Added photos to README"
git push
```

---

## Troubleshooting

### "fatal: not a git repository"
```powershell
# Make sure you're in the project folder
cd "C:\Users\Matt\Desktop\SuperDashv7"

# Initialize git
git init
```

### "remote: Repository not found"
- Verify repository URL is correct
- Check you're logged into correct GitHub account
- Ensure repository exists on GitHub

### "failed to push some refs"
```powershell
# Pull first (if repository has changes)
git pull origin main --allow-unrelated-histories

# Then push
git push
```

### "Large files detected"
If you accidentally added large files:

```powershell
# Remove from staging
git rm --cached path/to/large/file

# Add to .gitignore
echo "large_file.bin" >> .gitignore

# Commit
git commit -m "Removed large files"
```

### "Everything up-to-date"
This is normal! It means there are no new changes to push.

---

## GitHub Features to Enable

### 1. Issues
Good for tracking bugs and feature requests. Already enabled by default.

### 2. Discussions
Enable for community Q&A:
- Repository → Settings → General → Features
- Check "Discussions"

### 3. Actions (CI/CD)
For automated testing (advanced):
- Can auto-compile Arduino sketches on commit
- Can run documentation checks
- Set up later if needed

### 4. Releases
Create releases for stable versions:
```powershell
# Tag a version
git tag -a v1.0 -m "Version 1.0 - Initial release"
git push origin v1.0
```

Then create release on GitHub with release notes.

---

## Collaboration

### Allow Others to Contribute

**For public repositories:**
- Others can fork your repository
- They can submit pull requests with improvements
- You review and merge their changes

**For private repositories:**
- Repository → Settings → Collaborators
- Add specific GitHub users

### Creating a CONTRIBUTING.md

```markdown
# Contributing

Thanks for your interest in contributing!

## How to Contribute

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes
4. Test thoroughly
5. Commit: `git commit -m "Description of changes"`
6. Push: `git push origin feature-name`
7. Open a Pull Request

## Code Style

- Follow existing code formatting
- Comment complex sections
- Update documentation for new features
```

Save as `CONTRIBUTING.md` in root, then:
```powershell
git add CONTRIBUTING.md
git commit -m "Added contributing guidelines"
git push
```

---

## Success Checklist

After following this guide, verify:

- [ ] Repository visible on GitHub
- [ ] README.md displays correctly
- [ ] Folder structure is clean and organized
- [ ] LICENSE file present
- [ ] No planning documents visible
- [ ] No unnecessary files (build artifacts, logs)
- [ ] Can clone repository and everything works
- [ ] Documentation is readable on GitHub
- [ ] Images display correctly (if added)
- [ ] Repository has description and topics

---

## Clone Your Repository (Test)

To verify everything worked, try cloning to a different location:

```powershell
# Go to different folder
cd C:\Temp

# Clone your repository
git clone https://github.com/YOUR_USERNAME/esp32-vehicle-monitor.git

# Navigate into it
cd esp32-vehicle-monitor

# Verify contents
ls
```

You should see the organized structure ready to use!

---

## Next Steps

1. **Star your own repository** (why not! 😄)
2. **Share with community:**
   - Reddit: r/esp32, r/arduino
   - Arduino forums
   - ESP32 forums
   - Twitter/X with #ESP32 #Arduino
3. **Add to GitHub profile README**
4. **Consider writing a blog post about the project**
5. **Submit to Hackaday.io or Hackster.io**

---

**Congratulations! Your project is now on GitHub and ready to share with the world! 🎉**

---

**Last Updated:** January 2026  
**Difficulty:** Beginner-friendly  
**Time Required:** 15-30 minutes
