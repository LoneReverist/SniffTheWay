# Recurring location maps — draft v1

These are proposed continuity layouts for review, not reconstructions of exact geography from the old images. The user's story relationships are fixed requirements; stone count, coordinates, landmark placements, and camera positions below are proposed design decisions. No game assets have been replaced.

## Selected production references

[Final selected environment references](references/README.md): accepted same-side-tree creek pair and the original simplified night background. All night pages keep paths subtle and focus on baby/dog; maps specify connectivity without requiring paths to dominate the images.

## Map set

1. [Picnic, hedge and lost trail](01_picnic_map.svg)
2. [Creek crossing](02_creek_map.svg)
3. [Shelter tree and two-way trail](03_shelter_map.svg)
4. [House exterior](04_house_exterior_map.svg)
5. [Living room](05_living_room_map.svg)

The maps are editable SVG diagrams rather than generated paintings. [build_maps.py](build_maps.py) regenerates them. They define object relationships; later painted reference boards define materials and appearance.

## Reading the maps

Up is local map north; x increases right/east, y increases down/south, on a 0–100 layout grid. Coordinates describe composition space, not meters. Each outdoor map is its own local frame: arrows between locations establish narrative order, not a surveyed world compass or distance. The house exterior and interior share north/east orientation. Camera markers are viewpoint positions, arrows show aim; they are not field-of-view cones. Camera markers belong to different shots and never appear in artwork.

Travel order: picnic clearing → hedge gap → concealed trail bend → creek Bank A → S1 → S2 → S3 → Bank B → shelter's incoming path → shelter's forward path → house gate → porch/hall → living room. Do not introduce a second creek crossing on the homeward route. Distances between locations remain unspecified.

All maps use the selected simplified style only as a future rendering requirement. Blue night foliage, quiet moss patches and grouped ground leaves remain governed by the art guide. Map symbol colors are diagram keys, not production palette swatches.

## 1. Picnic and lost trail

Established: lake/mountain outlook, shade tree, blanket and basket; wind distracts the parents; baby follows butterflies and dog follows baby through a hedge to a nearby trail. Beyond the hedge they cannot see the way back.

Proposed layout: clearing north of a dense east–west hedge P-H. Blanket P-B at (39,42), shade tree P-T at (20,38). Hedge gap P-G at (49,66) remains physically open. The short connector runs south to (49,77), turns east around shrub P-S at (67,69), and then southeast past tree P-T2 at (77,76). The lost position is around (81,91). These two bends and foliage block the low eye-level view back; the hedge does not close or move.

Wind and parents head northeast from the blanket toward a retrieval area around (80,30), away from the baby/dog route. Keep retrieval on the meadow, not in the lake. Basket, blanket and mugs are movable props: record their original, airborne and landed positions; their disappearance after occlusion is not an environment change. Butterflies guide movement, not fixed landmarks.

| Shot | Camera → aim | Action and visible continuity |
|---|---|---|
| P1 / 1_1_picnic | (25,58) → (43,40), wide | Family at blanket; shade tree left, lake beyond clearing; hedge behind camera or peripheral. |
| P2 / 1_2_gust_of_wind | (30,60) → (61,37), wide | Parents pursue blanket northeast; baby turns toward gap, dog stays near baby. Same lake and tree bearing. |
| P3 / 1_3_following_butterflies | (60,85) → (49,66), medium-wide reverse | Baby just beyond gap around (51,74), dog nearer gap. Parents may be glimpsed distantly from this elevated camera through the gap, while baby looks away. Avoid showing parents clearly above a supposedly opaque hedge at infant eye height. |
| P4 / 1_4_lost | (96,96) → (79,87), low medium-wide | Baby/dog past the bend; trail behind recedes into foliage, not a visible opening onto the picnic. Preserve selected lost-scene emotion and framing motifs. |

Sightline test: trace a line from the baby at the lost mark to both blanket and retrieval area. It must intersect dense foliage. Do not simply erase parents from an otherwise unobstructed vista. The current lost painting is the style/composition reference; the hedge connector is a newly explicit part of its offscreen geography.

## 2. Creek

Established: characters approach one creek, dog leads onto stones, baby hesitates, follows, then both leave on the other bank. Existing wide views most clearly support three main stones. Adopt exactly THREE traversable stones in this proposal, with smaller bank rocks explicitly excluded from that count.

Bank A is south (entry), Bank B north (exit). Water flows west to east. Entry path connects at (48,68), exit path at (50,32). All stone tops are the same warm gray/tan material with broad clean surfaces, darker wet sides, and restrained moss below the walking surface. No dense cracked-earth pattern.

| ID | Center | Plan size, layout units | Fixed silhouette / orientation |
|---|---|---|---|
| S1 | (48,60) | 16 × 9 | Broad oval; long axis east–west. |
| S2 | (52,50) | 18 × 9 | Widest; shallow notch on EAST edge. The SVG ellipse marks its footprint; notch must be added in the material/shape board. |
| S3 | (50,40) | 16 × 9 | Rounded slab with flattened NORTH edge; footprint proxy shown in map. |

Stone centers, orientations and order stay fixed. Bank gaps and stone gaps are symbolic until a character-scale blockout verifies reach. Use broad tops large enough for the crawling baby's support points, short reachable gaps, and low, similar exposed heights. If scale tests fail, adjust the entire crossing map once, never a single shot's stone spacing. Avoid precise real-world dimensions until this test is done.

Landmarks: K-T1 entry-bank tree southwest (25,75); K-T2 exit-bank tree northwest (25,23); K-R broad boulder southeast (72,72); K-F flower patch northeast (74,25). User-selected revision: both large trees are WEST of the path, across the creek from each other. Approach shows both on the left; reverse shows both on the right. Boulder and flower patch occupy the more open east side. See [selected creek references](references/README.md). Ripples/wakes stretch downstream east. In the north-facing approach, flow projects roughly left-to-right; in south-facing reverse views it projects roughly right-to-left. Never flip a finished image to create a reverse angle.

| Shot | Camera → aim | Character blocking and stone visibility |
|---|---|---|
| K1 / 2_1_approaching_creek | (34,91) → (50,46), low wide north | Baby on approach around (44,81), dog at A lip (48,70). S1 nearest then S2 and S3 recede; B path beyond. |
| K2 / 2_2_baby_reluctant | (70,29) → (48,63), low oblique south | Dog testing S1; baby on A around (47,72). S3/S2 may enter foreground or crop out. Do not relabel the nearest visible stone as S1. |
| K3 / 2_3_crossing_creek | (59,23) → (51,51), low south | Baby supported on S1; dog moving from S2 toward S3. S3 foreground, S2 middle, S1 behind. Character limbs may hide edges; record occlusion rather than delete stones. |
| K4 / 2_4_beyond_the_creek | (48,8) → (53,33), medium-wide south | Both on B around (53,27), dog a little farther forward. Stones recede S3→S2→S1 toward A. Show entry-bank landmarks behind; do not reuse the B-side backdrop from K1. |

Light: evening as stated by story JSON, softer and lower than current bright midday-looking creek images. Within this short sequence keep the same world-space light direction, provisionally from west/southwest. Track modest paw/hand/trouser dampness after crossing without changing clothes.

## 3. Shelter tree

Use [the original simplified shelter background](references/shelter_background.png) as the selected environment reference. In ALL night pages, the dog and baby are the focal point; paths stay subtle, partly obscured and secondary. The map describes the route offscreen as well as onscreen. Do not force a prominent through-trail or intersection-like clearing into the artwork.

Established: one large rooted tree, several camera angles, incoming and forward paths, sleeping beneath its roots; moon above the path forward. Morning continues from the same shelter.

Proposed layout: trunk N-T centered (39,39), one hollow N-H opening SOUTH around (43,57). Two buttress roots frame the opening: west root ends (35,68), east root ends (63,67). This is one recess, not a tunnel and not a separate hollow on the opposite face. Main trail passes south of the tree, entering southwest, then curves northeast toward home. N-F purple flowers at (67,73), N-R rock (23,76), N-T2 slender trail-side tree (77,43) anchor reverse views. Broad root moss remains attached to the same sides.

| Shot | Camera → aim | Blocking / orientation |
|---|---|---|
| N1 / 3_1_dusk | (78,61) → (40,73), wide southwest | Characters arrive along southwest trail near (37,75); large tree can sit screen-right. Incoming path recedes behind them. Hollow is oblique, not a new front opening. |
| N2 / 3_2_exhausted | (74,77) → (43,69), close northwest | Baby sits just outside hollow around (40,69), dog beside him (48,68). Root edge and fixed flowers provide context; forward path may be cropped. |
| N3 / 3_3_night | (57,91) → (44,57), wide north/northwest | Baby sleeps on west side of recess, dog watches from east side. Trunk/root arch left, forward path on right. Match selected simplified night treatment. |
| N4 / 3_4_morning | (61,83) → (46,57), slightly closer north/northwest | Same resting patch and root arrangement; both awake, dog scents toward northeast trail. Flowers and path remain on the same world-space sides. |

Moon is a SKY BEARING toward the northeast opening, not an object sitting on the trail. Compose N3 with the moon visually above the forward path in the upper-right sky. In N1's reverse direction it should be behind camera/out of view, not copied above the incoming path. Its exact pixel location changes with camera and framing. Morning sun can come from the eastern opening but need not occupy the moon's former position. These are art-direction bearings, not a claim of astronomical timing.

Show the two directions as one continuous trail passing the tree, not an unexplained three-way junction. Leave home out of sight until the later approach. No new opening, root limb, tree rotation or flower-bed relocation between N1–N4.

## 4. Home exterior

Established: pale siding, dark roof, green shutters/garage door, central front porch, garden path, white fence/flowering arch, sign, green mailbox, and parents coming toward the baby and dog. The current 4_1 image also has an officer; this map does not decide character casting. If retained, keep that person near the porch and outside the reunion action.

Proposed layout: front faces south. Garage west; hall/porch center; living room east, with chimney on the east wall to match the interior fireplace. Fence crosses the south garden at y73, gate H-G at (49,73). Sign H-S west of the gate, mailbox H-M east and outside the fence. Keep the mailbox number 17 and sign design consistent if retained; environmental lettering needs its own legibility review.

Path leads from woodland south through gate to porch at (56,43). Reunion H-R around (53,59), inside the garden. Simplify the existing illustrations' ambiguous overlapping fences into one boundary crossed once. Flower beds flank the path; no new hedge blocks parents' sightline to the gate.

| Shot | Camera → aim | Blocking / framing |
|---|---|---|
| H1 / 4_1_home | (45,98) → (55,39), wide north | Baby/dog approach from south; parents near porch (56,45). Sign left, mailbox right, house beyond. |
| H2 / 4_2_parents_notice | (42,91) → (54,58), medium-wide north | Parents move south to reunion mark, children move north through gate. Keep porch, gate and mailbox fixed. |
| H3 / 4_3_reunion | (38,80) → (53,59), closer northeast | Mother embraces baby, father greets dog at H-R. House remains behind, sign/mailbox can leave crop naturally. |

Create one exterior elevation from this footprint before final generation: window/door count, roof gables, porch posts, chimney, garage and flower boxes must agree across all three images. The map deliberately does not invent the unseen rooms or floor count.

## 5. Living room

The interior sits in the east wing shown on the exterior map. Orientation is shared, but the interior map is enlarged and uses its own layout scale. Front door opens into a central hall; a west-side opening at (12,72) connects hall to living room. The two interior story shots face generally northeast, explaining the window/sofa screen-left and fireplace screen-right.

Fixed objects: L-W north window; L-S green sofa below it, facing south; side table/lamp northeast of sofa; bookshelf between table and east fireplace; L-F fireplace centered on east wall with wood mantel and dark curved guard; L-R rug in center; L-B bed at (63,60), outside the projecting hearth. Preserve a clear floor gap between bed and hearth in the scale blockout. Mantel decor, pillows, throws and baskets get fixed IDs in the later prop board; do not regenerate a different room for the close-up.

| Shot | Camera → aim | Blocking / framing |
|---|---|---|
| I1 / 4_4_safe_again | (25,82) → (56,44), wide northeast | Family seated together on central rug around (46,54), with book. Bed stays to their east and may be partially visible or outside crop. Window/sofa left, fireplace right. |
| I2 / 4_5_by_the_fireplace | (43,77) → (64,57), low tighter northeast | Baby and dog sleep in the same fixed bed. Camera moves closer; fireplace remains behind/right and sofa edge left. Parents may be offscreen. Bed does not slide up onto hearth for composition. |

Daylight comes from north window in I1; fire/lamp contribute warm local light. I2 can emphasize firelight without rotating furniture or inventing another window. Treat any elapsed time as a lighting change, not a room change.

## What to build next

1. Review these proposed layouts, especially the three-stone choice, hedge sightline break, and one south-facing shelter hollow.
2. Make simple character-scale blockouts: creek first, then root hollow, then gate/room. Use the preferred larger-dog lineup. Test stone gaps, root clearance, furniture spacing and all camera views.
3. Save one master map per location plus landmark shape/material boards. Creek needs actual S1/S2/S3 silhouettes; shelter needs trunk/root front and side elevations; house needs facade elevation and room wall views.
4. Render flat camera guides from those fixed layouts, then generate backgrounds in the selected simplified treatment. Add characters afterward using identity and size references.
5. Review sequences together with actual story text/decorations from the [scene inventory](../scene_inventory.md). Current JSON placement is part of composition testing; no map assumes every shot has an empty upper margin.

## Acceptance checks

Every story image has a camera above. Hidden/cropped objects remain present in the map. Entry/exit banks, numbered stones, flow and character order survive reverse views. The picnic becomes hidden through physical occlusion. Shelter paths connect to the same tree and the moon belongs to the forward bearing. Exterior chimney and interior fireplace agree. Furniture never relocates between shots. All coordinates and camera aims remain provisional until blockout review; these maps do not promise pixel-identical copies of inconsistent old backgrounds.
