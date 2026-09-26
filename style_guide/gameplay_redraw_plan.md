# Gameplay background redraw plan

Prepared 2026-09-18. Planning only: no gameplay images or JSON changed.

## Objective and precedence

Repaint all 22 gameplay backgrounds in the selected simplified painterly style while retaining the existing playable composition. Include the separate `2_fallen_tree_log.png` overlay as a dependent asset (23 image assets total).

The current gameplay image controls composition; its gameplay JSON controls camera and interaction geometry. The style guide controls painting treatment, palette relationships and detail density. Story environment references supply materials and atmosphere, never replacement camera angles or path layouts. Historical story maps must not override calibrated gameplay geometry.

This is a controlled repaint, not a new interpretation of each scene. Keep source images and JSON as immutable baselines. Do not adjust JSON to make a drifting candidate fit.

## Findings from the game

- `resources/gameplay/` contains 22 scene definitions, each referencing a corresponding background.
- `src/Background.ixx` maps the whole background texture onto the UI rectangle; `src/SniffTheWayConstants.ixx` defines a 1920 × 1080 design viewport fitted inside the framebuffer.
- `src/Camera.ixx` uses a perspective camera with Z up. The JSON bounds and trigger coordinates are ground-plane world coordinates, not image percentages. Overlay generation must use the actual camera/view/projection and viewport conventions.
- Scene JSON also defines scent trails, scene-link triggers and arrival points, message triggers, and character tint. Preserve all of these, not just camera and bounds.
- `2_fallen_tree` includes an independently rendered log texture with full-image `image_rect` and depth 3.0, plus on-enter triggers. Its background already depicts the log: check the assembled scene and preserve the overlay/background relationship and silhouette.
- Inspected representative originals: playground, golden intersection, fallen tree and house path. Complete per-image visual audits remain part of preparation below.

## 1. Freeze and annotate all originals

Create `style_guide/gameplay/` with `manifest`, `guides`, `candidates`, `review`, and `approved` folders. Preserve exact source filename, dimensions, alpha/channel requirements, color profile and file hashes in a manifest, together with the source JSON hash. Record referenced overlay textures separately.

For every scene, capture an unmodified in-game baseline and a separate guide view showing bounds, scent trails, link/message triggers, arrival positions, and representative dog/baby placements at near, middle and far distances. Prefer existing editor rendering; if an export helper is needed, mirror the actual renderer and verify its projection against editor screenshots first. Never directly plot world XY as image XY. Clip behind-camera and offscreen segments correctly.

On the image, trace the visible path margins and record a small set of normalized landmark anchors: entry and exit locations, bends/forks, horizon/vanishing region, tree bases, boulder edges, shore edges, fence openings and prop footprints. JSON bounds need not coincide exactly with painted path edges; preserve the original relationship and safety margins rather than assuming they should coincide. Note existing discrepancies separately.

## 2. Establish a repeatable repaint recipe

Use each original as the primary edit target, plus one selected style reference matched to the time of day. Keep guide overlays separate from final artwork. The same original must remain the structural reference for every retry; avoid long chains of regenerations that accumulate drift and texture noise.

Lock framing, aspect ratio, perspective, horizon, path centerline and width at all depths, fork angles, exits, blocking silhouettes and major landmark positions. Do not crop, zoom, mirror, rotate, widen trails, reveal new routes or move objects. Do not bake dog, baby, scent trails, UI or editor guides into backgrounds.

Change rendering inside those shapes: broad foliage masses, quieter grass, grouped flowers, restrained bark/moss, broad stone surfaces and clean value separation. Reduce isolated bright flecks and uniform micro-detail without blurring structural edges. Retain natural grass between stones where already present. Use the selected daytime and shelter style references; preserve each original's time of day and light direction. Evening-to-dark and morning-to-home scenes should form coherent progressions rather than receiving one universal color wash.

Evaluate backgrounds with current character tint values and actual sprites. Use SDR sRGB output; preserve exact source pixel dimensions for delivery, with no crop or anisotropic stretch to accommodate generation output. If a tool cannot preserve the framing closely enough, use localized repainting and compositing onto the original geometry; do not compensate by editing the camera.

## 3. Pilot before producing the set

1. `1_forest_path`: standard receding path and daytime treatment.
2. `2_golden_intersection`: fork geometry, three scene links, golden light.
3. `2_dark_forest2`: dark palette and route readability approaching the night story.
4. `2_fallen_tree` plus log overlay: layered obstacle alignment and character occlusion.

Finish a style review AND gameplay alignment check for these before scaling up. If the repaint process cannot retain their structure, change the method before generating the remaining images. A pleasing illustration alone is not a pass.

## 4. Production inventory and order

After pilots, work in small batches of 3–4, comparing neighbors together. Every listed ID maps to `resources/gameplay/<id>.json` and `resources/textures/gameplay_backgrounds/<id>.png`.

| Sequence | Scene IDs in travel order | Main constraints to check |
|---|---|---|
| Early forest | `1_forest_path`, `1_forest_path2`, `1_right_turn` | Perspective narrowing, bend and exits; picnic transition |
| Lake and playground | `1_forest_lake`, `1_playground`, `1_forest_horizontal` | Shore/path boundary, playground access and props, lateral travel corridor |
| Creek approach | `1_thicker_forest_transition`, `1_before_creek` | Narrow corridor, forest density, creek-story transition |
| Evening routes | `2_after_creek`, `2_golden_intersection`, `2_golden_path`, `2_golden_hour` | Existing branch positions and destinations, consistent evening progression |
| Obstacle branch | `2_fallen_tree` and `2_fallen_tree_log.png` | Dead-end/return link, log footprint, alpha silhouette and depth occlusion |
| Deep forest | `2_deep_forest`, `2_dark_forest`, `2_dark_forest2` | Route visibility without excessive glow, all turns and exit positions |
| Morning forest | `3_morning_forest`, `3_morning_forest2`, `3_dirt_path` | Shelter-to-morning continuity, dirt corridor geometry |
| Home approach | `3_dirt_intersection`, `3_bench_path`, `3_house_path` | Junction/bench positions, fence and village silhouettes, stone-path perspective |

For house path, use the home story's palette/material language without replacing the existing village view with the front-yard story composition. For creek-adjacent scenes, do not introduce or relocate stepping stones merely to imitate a story reference.

## 5. Acceptance checks for every candidate

1. Compare source and repaint with a blink view and 50% overlay in the same frame. Assess landmark alignment, path edges and perspective before surface style. Pixel-difference scores alone are unsuitable because recoloring intentionally changes most pixels.
2. Overlay the SAME projected gameplay data on both. Check every route, all arrival positions, trigger crossings and branch exits. No new visual obstacle inside the traversable corridor, and no visually inviting new path beyond the original permitted area.
3. Proposed initial screening tolerance: critical path/obstacle/exit anchors within 0.25% of frame width/height (about 5 × 3 design pixels at 1920 × 1080); secondary noninteractive landmarks within 0.5%. These are review targets to validate in pilots, not engine guarantees. Tighten wherever existing clearance is smaller. Any changed collision/transition meaning fails regardless of numerical tolerance.
4. Run the game with unchanged JSON. Walk both corridor edges and centerline; test near/mid/far character scale, every link in both available directions, message triggers, scent visibility, shadows and obstacle occlusion. Test the fallen log in particular with characters in front and behind its render depth.
5. Compare adjacent scenes and story transitions for lighting, color family and detail density. Check readability with actual character tint, HUD and messages at normal and small display sizes. Verify representative desktop/mobile/TV display conditions when available; do not claim matching displays from palette choice alone.
6. Confirm exact output dimensions and alpha requirements, complete files, unchanged JSON hashes and no guide marks or baked-in characters. Record pass/fail and any remaining differences per scene.

Reject or locally repair layout drift instead of accepting it because the new art looks better. If an original has a gameplay mismatch, record it as separate work rather than silently changing this redraw's scope.

## 6. Delivery and integration

Keep versioned candidates and their exact prompts/reference lists. Put accepted images under `style_guide/gameplay/approved/` using the original names. The review manifest should identify source, chosen candidate, structural review, style review, in-game verification and overlay dependencies.

After the set is accepted, replace the matching gameplay textures together with required overlays, retaining a recoverable baseline. Leave scene JSON, camera values, bounds, triggers, arrival positions, tints and render placement unchanged. Run the full route and obstacle branch once more after integration and check the resource diff contains only intended assets. Integrating files is not evidence of an in-game pass; record which checks actually ran.

## Reusable prompt skeleton

“Repaint this exact gameplay background in the selected simplified painterly storybook style. Image 1 is the immutable layout authority; image 2 controls rendering treatment only. Preserve the entire frame, perspective, horizon, path margins and widths at every depth, bends, junctions, entrances/exits, and all listed landmark silhouettes and locations. Simplify detail inside existing forms with broad foliage groups, quiet ground textures and restrained highlights. Preserve original light direction and time of day. No characters, interface, added objects or new routes. No crop, zoom, camera movement or scene redesign. Scene-specific locked anchors: [guide].”

Next concrete action: build the source manifest and projection/landmark guides, then repaint `1_forest_path` as the first pilot. This document does not authorize or begin a bulk generation run.
