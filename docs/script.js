(() => {
  const root = document.documentElement;
  const themeSelect = document.querySelector("#heim-theme-select");
  const colorScheme = window.matchMedia("(prefers-color-scheme: dark)");
  const themeStorageKey = "heim-theme";
  const validThemes = new Set(["system", "light", "dark"]);

  function getSavedTheme() {
    const savedTheme = localStorage.getItem(themeStorageKey);

    if (validThemes.has(savedTheme)) {
      return savedTheme;
    }

    return "system";
  }

  function resolveTheme(theme) {
    if (theme === "system") {
      return colorScheme.matches ? "dark" : "light";
    }

    return theme;
  }

  function applyTheme(theme) {
    root.dataset.heimTheme = resolveTheme(theme);
  }

  let currentTheme = getSavedTheme();
  applyTheme(currentTheme);

  if (themeSelect !== null) {
    themeSelect.value = currentTheme;

    themeSelect.addEventListener("change", () => {
      const selectedTheme = themeSelect.value;

      if (!validThemes.has(selectedTheme)) {
        return;
      }

      currentTheme = selectedTheme;
      localStorage.setItem(themeStorageKey, currentTheme);
      applyTheme(currentTheme);
    });
  }

  colorScheme.addEventListener("change", () => {
    if (currentTheme === "system") {
      applyTheme(currentTheme);
    }
  });

  const githubLink = document.querySelector("[data-heim-github-repo]");

  if (githubLink !== null) {
    const repository = githubLink.dataset.heimGithubRepo;
    const starCount = githubLink.querySelector(".heim-github__count");

    if (repository !== undefined && repository.length > 0) {
      githubLink.href = `https://github.com/${repository}`;

      fetch(`https://api.github.com/repos/${repository}`, {
        headers: {
          Accept: "application/vnd.github+json"
        }
      })
        .then((response) => {
          if (!response.ok) {
            throw new Error(`GitHub API returned ${response.status}`);
          }

          return response.json();
        })
        .then((data) => {
          if (starCount === null) {
            return;
          }

          const stars = Number(data.stargazers_count);

          if (!Number.isFinite(stars)) {
            throw new Error("Invalid GitHub star count");
          }

          const formatter = new Intl.NumberFormat("en", {
            notation: stars >= 1000 ? "compact" : "standard",
            maximumFractionDigits: 1
          });

          starCount.textContent = formatter.format(stars);

          githubLink.setAttribute(
            "aria-label",
            `Heim on GitHub — ${stars} stars`
          );
        })
        .catch(() => {
          if (starCount !== null) {
            starCount.textContent = "—";
          }
        });
    }
  }
})();
