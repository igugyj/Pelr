# Contributing to Pelr

Pelr welcomes contribution of any size — from users, not only C++ developers.

## Ways to Contribute

| Type | What to do | Where |
| --- | --- | --- |
| **Code** | Fix bugs, add features, refactor, optimize | Pick a task from [Issues](https://github.com/igugyj/Pelr/issues), or open an Issue first to discuss the approach |
| **Documentation** | Improve `docs/` user guides, add screenshots/screen recordings, fix typos and stale descriptions | Send a PR touching `docs/*.md` |
| **Translation** | UI strings in `translations/*.ts` (zh_CN / en_US), or translate documentation | Send a PR, or state in an Issue which languages you can cover |
| **Ideas** | Feature suggestions, UX improvements, roadmap discussion | [Feature Request template](https://github.com/igugyj/Pelr/issues/new/choose) |
| **Testing** | Try the pre-release builds on [Releases](https://github.com/igugyj/Pelr/releases) and report bugs with logs | Follow the bug template in [SUPPORT](SUPPORT.md) |
| **Ecosystem** | Packaging, mirrors, components, third-party maintenance | Claim a task in an Issue |

> Small changes (docs, translations, typos): open a PR directly. Architecture or new features: open an Issue and reach agreement first, so the work is not thrown away.

## Getting Started

1. Fork the repository and clone it locally
2. Branch off `dev` (never off `master`):

   ```sh
   git checkout dev
   git pull origin dev
   git checkout -b feature/your-feature-name
   ```

3. Follow the [development guide](docs/dev-guide.md) to set up the toolchain, third-party dependencies and build
4. Make your change, test it locally, then open a PR against `dev`

## Project Layout

- [docs/dev-guide.md](docs/dev-guide.md) — environment setup, dependencies, Debug/Release builds, release scripts and Release CI
- [docs/dev-structure.md](docs/dev-structure.md) — source tree overview
- [NOTICE](NOTICE) — third-party dependencies and licenses

Quick reminders:

- Sources are collected with `file(GLOB ...)` — **re-run cmake configure after adding a `.cpp`/`.h`**
- The build type decides `DEBUG_MODE` (Release → OFF); do not hand-edit `CMakeLists.txt`
- Release builds intentionally ship without Live2D Core / voicevox components; starting in a "component missing" state is by design
- Branch model: day-to-day work lands in `dev`, only stable releases merge into `master`

## Coding Guidelines

- **Language:** C++17
- **Naming:** `PascalCase` for classes, `camelCase` for variables and functions, `UPPER_SNAKE_CASE` for constants and macros
- **Indentation:** 4 spaces; K&R braces; comments in Chinese or English, kept concise
- **Include order:** project headers → Qt headers → standard library
- **Qt / OpenGL:** multiply mouse coordinates by `devicePixelRatioF()`; clear the framebuffer each frame with `glClearColor(0,0,0,0)` + `glClear(GL_COLOR_BUFFER_BIT)` to keep the background transparent; use `grabFramebuffer()` for pixel reads outside `paintGL`
- **compatLApp shadow layer:** `src/compatLApp/` overrides same-named files in `thirdParty/CubismNativeSamples/Samples/Common/` — **never edit the originals under `thirdParty/`** (they are excluded from the build)
- **Never remove** the globally forced `-include GL/glew.h` or `GLEW_STATIC`

## Pull Request Process

1. Base your branch on the latest `dev`
2. Keep commits focused and single-purpose, messages follow Conventional Commits: `type(scope): description` (`feat` / `fix` / `refactor` / `docs` / `style` / `chore` / `build` / `test`)
3. Build locally: `cmake --build build/Debug`
4. Update the matching `docs/` page when behavior changes
5. Target the `dev` branch and fill in the PR template self-check list
6. Example: `feat(core): add mouse transparency check on window activate`

## Reporting Issues

- Use an [Issue template](https://github.com/igugyj/Pelr/issues/new/choose) (bug / feature, English and Chinese)
- For bugs include: reproduction steps, Pelr version, Windows version, logs from `log/`, screenshots
- Check [SUPPORT](SUPPORT.md) and search existing Issues first
- **Never report security vulnerabilities publicly** — see [SECURITY](SECURITY.md)

## License & Legal

- Author-written code under `src/` is MIT; the project as a whole is governed by the licenses listed in [NOTICE](NOTICE)
- Live2D Cubism Core is proprietary and must not be redistributed; if you distribute compiled binaries, obtain a Live2D publishing license yourself ([terms](https://www.live2d.com/en/sdk/license/))
- `thirdParty/LAppLive2D` and `src/compatLApp` derive from `CubismNativeSamples` and follow the Live2D Open Software License

Please read our [Code of Conduct](CODE_OF_CONDUCT.md). Thanks to every contributor!
