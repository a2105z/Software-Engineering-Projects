# Enable GraphForge on GitHub Pages

Your GraphForge app is ready to deploy. Enable it with **one** of these options:

## Option 1: Deploy from branch (simplest)

1. Go to **https://github.com/a2105z/Software-Engineering-Projects/settings/pages**
2. Under **Build and deployment** → **Source**, choose **Deploy from a branch**
3. **Branch:** `main`
4. **Folder:** `/docs`
5. Click **Save**

Within a minute, your site will be live at:

**https://a2105z.github.io/Software-Engineering-Projects/**

---

## Option 2: GitHub Actions

1. Go to **https://github.com/a2105z/Software-Engineering-Projects/settings/pages**
2. Under **Source**, choose **GitHub Actions**
3. The workflow runs on every push to `main` that changes `docs/`
