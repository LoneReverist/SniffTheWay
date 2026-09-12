# Art direction — simplified painterly style selected

## Selected treatment
User decision, 2026-09-12: use the simplified style across the game. [Reference roles and review notes](style_selection.md) define which aspects of each candidate to follow. Palette swatches below remain provisional; the selected images guide the painting treatment.

## Visual language
Warm, gently dimensional children's storybook painting with the broad shapes and restrained surface detail of the simplified candidates. Softly modeled faces and bodies, clear silhouettes, broad painted foliage masses, selective clean edges. Inviting natural environments with enough depth to feel explorable. Express emotion through pose, brows, and gaze.

Keep from the lost reference: cream/gold dog against cool greens, blue clothing as a focal accent, warm stone path, framing trees, receding layers, readable worried faces.
Reduce: uniformly sharp foliage, tiny yellow flecks, isolated bright grass tips, repeated fur curls, granular bark, and countless equally distinct flowers. Avoid plastic 3D rendering, photographic surface texture, heavy outlines, and an overall yellow color cast.

## Detail and edge hierarchy
1. Faces, hands/paws, and the action: clearest shapes and selected crisp edges.
2. Nearby supporting objects: broad forms with restrained texture.
3. Background: grouped foliage and softer, lower-contrast layers.

Describe positive construction in prompts: large interlocking leaf masses, broad brush shapes, quiet shadow planes, sparse grouped flowers, a few curved fur clumps. Negative instructions alone are insufficient. Do not fix noisy outputs merely by blurring everything: repaint or regenerate from the approved clean reference.

Judge details at final presentation size. No scattered one-pixel highlights or stippling that competes with faces. At a roughly 400-pixel-wide thumbnail the action and expressions should remain readable. Inspect native pixels as well: softness at small size must not hide malformed hands, eyes, or repeated texture.

## Night treatment

Primary environment: [selected shelter background](environments/references/shelter_background.png). Across all four night pages, prioritize dog/baby emotion and the protective tree. Keep paths low contrast, narrow or partly hidden; do not expose both directions just to explain geography. The forward route should be suggested, not the main composition.
Use [night candidate 1](candidates/night_v1/01_simplified_final.png) as the primary reference. Keep bluish/teal leaf masses that belong to the nighttime atmosphere, restrained warm shelter light, and readable characters against cool surroundings. Avoid the overly bright lighting of night candidate 3. Do not brighten the whole scene to improve readability; preserve the night mood and separate the important forms locally.

Simplify moss on trunk and root edges into broad, quiet patches. Avoid densely articulated moss tips or a finely textured bright fringe around the shelter. Candidate 3's edge moss is a negative example.

The [ground leaves beneath the characters in candidate 3](candidates/night_v1/03_selective_final.png) are a positive secondary reference: readable painted leaf shapes grouped into the ground plane. Carry that treatment into future simplified scenes while retaining candidate 1's lighting and overall detail restraint. This is a ground-material preference, not approval of candidate 3 as a whole.

## Characters
Dog: small cream/apricot curly-coated dog; floppy golden ears; dark rounded nose; large brown eyes; curled plume tail; blue collar and round gold tag. Preserve muzzle length and head/body proportions from the sheet. Represent curls as larger grouped locks at scene scale.

Baby: short warm brown hair with front tuft, brown eyes, rounded cheeks, blue short-sleeved shirt, gray trousers, bare feet. Preserve infant proportions and crawling ability. Sitting and sleeping poses are appropriate; do not accidentally age him into a walking toddler. Existing night images appear to introduce footwear: resolve this in favor of the supplied sheet unless a change is requested.

Parent sheets and a shared scale lineup are available in [characters/](characters/README.md), with the user-selected first lineup and its larger dog. Keep the dog a little taller than the baby as shown there; the later reduction and earlier numeric dog targets are superseded. Do not infer scale independently in each shot. Keep parent clothing and identity consistent with those sheets; use simplified scene references for texture density.

## Proposed palette roles
These are starting swatches, not extracted or approved colors. Lighting may shift them while preserving identity.

| Role | sRGB hex | Use |
|---|---|---|
| Cream | #F0D6A5 | Dog lit fur |
| Apricot | #C99759 | Ears and fur shadow accents |
| Character blue | #277EAF | Shirt and collar |
| Leaf green | #6F884B | Main foliage |
| Deep green | #344D42 | Woodland shade |
| Warm stone | #B8A17B | Path and creek stones |
| Water | #648E96 | Muted creek midtones |
| Night blue | #354D70 | Night atmosphere |
| Warm light | #E5BD79 | Sunlight/firelight accents |

Use a common palette family across the story, not identical brightness in every scene. Day: warm light and cooler green shadows. Evening creek: lower, softer warm light matching the narrative. Night: blue atmosphere with readable faces and distinct silhouettes. Morning: fresh warmth on the same shelter. Interior: warm local fire/lamp light with quieter neutral shadows.

## Display and composition checks
Use an SDR sRGB delivery workflow; the game currently loads these backgrounds as RGBA_SRGB. Keep a color-managed master and convert to sRGB rather than merely relabeling another color space. Verify export and in-game appearance together.

Avoid relying on very saturated greens/blues, nearly black shadow differences, or near-white highlight differences to convey essential information. Maintain value separation between characters and surroundings. Palette choices cannot guarantee matching appearances on uncalibrated displays; validate representative hardware.

Check each approved pilot on desktop, a phone, and a TV if available, at normal and reduced brightness. Examine night shadow readability, cream fur highlights, skin hue, and saturated blues. Review a grayscale proof and a small thumbnail. Simulation helps triage; it does not replace real displays.

Existing images are 1672 × 941 (approximately 16:9). Confirm final export size against the renderer before production; proposed standard is 1920 × 1080, with larger masters only if useful. Do not stretch to fix aspect ratios. Preserve filenames on final integration.

Text and decorations are rendered separately. Reserve quiet areas using actual per-page overlay locations, not a universal top margin. Never generate narrative text into the artwork. Review actual game overlays, transitions, and supported viewport behavior before replacing assets.
