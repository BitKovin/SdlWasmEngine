#include "Quake3Bsp.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <sstream>
#include <string>
#include <unordered_map>

// ── Internal helpers ──────────────────────────────────────────────────────────

#define PORTAL_SWAP_YZ

namespace {

    // Strip a trailing \r so we handle both \r\n and \n line endings.
    static void StripCR(std::string& s)
    {
        if (!s.empty() && s.back() == '\r')
            s.pop_back();
    }

    // True if every character in str is an ASCII digit (0-9).
    static bool IsAllDigits(const std::string& str)
    {
        if (str.empty()) return false;
        for (char c : str)
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    }

    // Convert a Quake-map-space position to engine space.
    // Divides every component by MAP_SCALE.
    // If PORTAL_SWAP_YZ is defined also applies Q3-standard (x,z,-y) axis swizzle.
    static glm::vec3 ToEngineSpace(float x, float y, float z)
    {
#ifdef PORTAL_SWAP_YZ
        return { x / MAP_SCALE, z / MAP_SCALE, -y / MAP_SCALE };
#else
        return { x / MAP_SCALE, y / MAP_SCALE, z / MAP_SCALE };
#endif
    }

    // Parse the vertex list at the current position of 'ls'.
    // Expected token format: (x y z)  — parentheses act as delimiters.
    // Returns false if fewer than numVerts could be read.
    static bool ParseVertices(std::istringstream& ls,
        int                 numVerts,
        std::vector<glm::vec3>& out)
    {
        out.reserve(numVerts);
        for (int v = 0; v < numVerts; ++v)
        {
            char  open, close;
            float x, y, z;
            if (!(ls >> open >> x >> y >> z >> close))
                return false;
            out.emplace_back(ToEngineSpace(x, y, z));
        }
        return (int)out.size() == numVerts;
    }

    // Compute the arithmetic centroid of a convex polygon.
    static glm::vec3 ComputeCenter(const std::vector<glm::vec3>& verts)
    {
        glm::vec3 c(0.f);
        for (const auto& v : verts) c += v;
        return c / static_cast<float>(verts.size());
    }

    // Build a BoundingBox from a list of points.
    static BoundingBox ComputeBounds(const std::vector<glm::vec3>& verts)
    {
        glm::vec3 bMin = verts[0];
        glm::vec3 bMax = verts[0];
        for (const auto& v : verts)
        {
            bMin = glm::min(bMin, v);
            bMax = glm::max(bMax, v);
        }
        return BoundingBox(bMin, bMax);
    }

    // Build a mapping from BSP cluster index → first leaf that belongs to it.
    // Multiple leaves can share a cluster in Q3 BSP; we pick the first one.
    static std::unordered_map<int, int>
        BuildClusterToLeafMap(const std::vector<tBSPLeaf>& leafs)
    {
        std::unordered_map<int, int> m;
        m.reserve(leafs.size());
        for (int i = 0; i < static_cast<int>(leafs.size()); ++i)
        {
            const int c = leafs[i].cluster;
            if (c >= 0)
                m.emplace(c, i);   // emplace keeps the first insertion
        }
        return m;
    }

    // ── String-pulling (funnel algorithm) helpers ─────────────────────────────
    //
    // The funnel algorithm produces the geometrically shortest path through a
    // sequence of convex portal polygons. This is the 3D version adapted for
    // Q3 BSP portals:
    //   • Left / right gate vertices are found by projecting the portal polygon
    //     onto the axis perpendicular to (apex → portal centroid) in the XZ plane
    //     (Y-up after the Q3 axis swap).
    //   • TriArea2D gives signed 2D orientation in XZ — sufficient for the funnel
    //     cross-product tests, and correct for Q3's mostly-horizontal geometry.
    //   • Vertical portals (doors spanning height only) degenerate gracefully.
    // ─────────────────────────────────────────────────────────────────────────

    // 2D signed triangle area in the XZ plane.
    //   Positive (CCW): C is to the LEFT  of directed edge A→B.
    //   Negative  (CW): C is to the RIGHT of directed edge A→B.
    inline float TriArea2D(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
    {
        return (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
    }

    // For a convex portal polygon and the current funnel apex, returns the gate:
    //   first  = LEFT  extreme vertex (most-CCW from the apex's perspective)
    //   second = RIGHT extreme vertex (most-CW)
    //
    // The gate is recomputed lazily using the current apex after each funnel
    // restart, so orientation is always relative to the live apex position.
    static std::pair<glm::vec3, glm::vec3> FindPortalGate(
        const std::vector<glm::vec3>& verts,
        const glm::vec3& apex)
    {
        const int n = static_cast<int>(verts.size());

        // Compute XZ centroid to derive the forward direction.
        float cenX = 0.f, cenZ = 0.f;
        for (const auto& v : verts) { cenX += v.x; cenZ += v.z; }
        cenX /= static_cast<float>(n);
        cenZ /= static_cast<float>(n);

        const float fwdX = cenX - apex.x;
        const float fwdZ = cenZ - apex.z;
        const float fwdLen = std::sqrt(fwdX * fwdX + fwdZ * fwdZ);

        if (fwdLen < 1e-5f)
        {
            // Apex is directly above/below the centroid. Return two spatially
            // distinct vertices so the funnel has something to work with.
            for (int i = 1; i < n; ++i)
            {
                const float dx = verts[i].x - verts[0].x;
                const float dz = verts[i].z - verts[0].z;
                if (std::abs(dx) + std::abs(dz) > 1e-4f)
                    return { verts[0], verts[i] };
            }
            return { verts[0], verts[0] };
        }

        // Right-axis in XZ: rotate forward 90° clockwise → (fwd.z, 0, -fwd.x).
        const float rX = fwdZ / fwdLen;
        const float rZ = -fwdX / fwdLen;

        int   leftIdx = 0, rightIdx = 0;
        float minP = FLT_MAX, maxP = -FLT_MAX;

        for (int i = 0; i < n; ++i)
        {
            // Signed projection onto the right axis (relative to apex)
            const float p = (verts[i].x - apex.x) * rX + (verts[i].z - apex.z) * rZ;
            if (p < minP) { minP = p; leftIdx = i; }
            if (p > maxP) { maxP = p; rightIdx = i; }
        }

        return { verts[leftIdx], verts[rightIdx] };
    }

    // Centripetal Catmull-Rom spline smoothing.
    //
    //   subdiv  Number of interpolated points inserted between each consecutive
    //           waypoint pair.  0 → no-op (returned path is unchanged).
    //   alpha   Tension parameter: 0.5 = centripetal (default, avoids cusps),
    //           0.0 = uniform, 1.0 = chordal.
    //
    // The first and last waypoints are always preserved exactly.
    static std::vector<glm::vec3> SmoothCatmullRom(
        const std::vector<glm::vec3>& pts,
        int   subdiv,
        float alpha)
    {
        const int n = static_cast<int>(pts.size());
        if (n < 2 || subdiv <= 0) return pts;

        std::vector<glm::vec3> out;
        out.reserve(static_cast<size_t>(n) + static_cast<size_t>(n - 1) * subdiv);
        out.push_back(pts[0]);

        for (int i = 0; i < n - 1; ++i)
        {
            // Clamp ghost control points at both ends so the spline still
            // reaches the first and last waypoints.
            const glm::vec3& p0 = pts[std::max(0, i - 1)];
            const glm::vec3& p1 = pts[i];
            const glm::vec3& p2 = pts[i + 1];
            const glm::vec3& p3 = pts[std::min(n - 1, i + 2)];

            // Insert `subdiv` intermediate samples at t = s/(subdiv+1).
            for (int s = 1; s <= subdiv; ++s)
            {
                const float t = static_cast<float>(s) / static_cast<float>(subdiv + 1);
                const float t2 = t * t;
                const float t3 = t2 * t;

                // Catmull-Rom basis with tension alpha:
                //   p0: -a*t³  + 2a*t²  - a*t
                //   p1: (2-a)*t³ + (a-3)*t² + 1
                //   p2: (a-2)*t³ + (3-2a)*t² + a*t
                //   p3:  a*t³  - a*t²
                out.push_back(
                    ((-alpha) * t3 + 2.f * alpha * t2 - alpha * t) * p0
                    + ((2.f - alpha) * t3 + (alpha - 3.f) * t2 + 1.f) * p1
                    + ((alpha - 2.f) * t3 + (3.f - 2.f * alpha) * t2 + alpha * t) * p2
                    + (alpha * t3 - alpha * t2) * p3
                );
            }

            // Emit the exact waypoint at t = 1 (except for the final segment —
            // pts.back() is appended after the loop to avoid a duplicate).
            if (i + 1 < n - 1)
                out.push_back(p2);
        }

        out.push_back(pts.back());
        return out;
    }

} // anonymous namespace



/// Parses a .prt file passed as a raw string and fills CQuake3BSP::portals.
///
/// @param content  Full text content of the .prt file (any line ending).
void CQuake3BSP::LoadPortalsFromPRT(const std::string& content)
{
    portals.clear();

    std::istringstream ss(content);
    std::string        line;

    // ── Header line 1: magic ─────────────────────────────────────────────────
    if (!std::getline(ss, line)) return;
    StripCR(line);
    if (line.rfind("PRT1", 0) != 0)
    {
        // Could add PRT2 / PRTX handling in the future if needed.
        assert(false && "LoadPortalsFromPRT: unsupported portal file format (expected PRT1)");
        return;
    }

    // ── Header line 2: numClusters ───────────────────────────────────────────
    if (!std::getline(ss, line)) return;
    StripCR(line);
    const int numClusters = std::stoi(line);
    (void)numClusters; // informational; not needed for parsing

    // ── Header line 3: numInteriorPortals ────────────────────────────────────
    if (!std::getline(ss, line)) return;
    StripCR(line);
    const int numInteriorPortals = std::stoi(line);

    // ── Header line 4: numLeafPortals (optional in some compilers) ───────────
    // Detect by peeking: if the next line is a pure integer it is part of the
    // header; otherwise we have already reached Section 1 and we put it back.
    int numLeafPortals = 0;
    {
        const auto savedPos = ss.tellg();
        if (std::getline(ss, line))
        {
            StripCR(line);
            // Trim leading whitespace
            const size_t first = line.find_first_not_of(" \t");
            const std::string token = (first == std::string::npos)
                ? "" : line.substr(first, line.find_first_of(" \t", first) - first);
            if (IsAllDigits(token))
            {
                numLeafPortals = std::stoi(token);
                // consumed successfully — leave stream pointer advanced
            }
            else
            {
                // Not a header line — rewind so it is parsed as portal data.
                ss.seekg(savedPos);
            }
        }
    }

    // Reserve space up front; Section 2 portals are appended after Section 1.
    portals.reserve(numInteriorPortals + numLeafPortals);

    // ── Cluster → leaf mapping ────────────────────────────────────────────────
    const auto clusterToLeaf = BuildClusterToLeafMap(leafs);

    auto resolveLeaf = [&](int cluster) -> int
        {
            const auto it = clusterToLeaf.find(cluster);
            return (it != clusterToLeaf.end()) ? it->second : -1;
        };

    // ─────────────────────────────────────────────────────────────────────────
    // Section 1 — Interior (cluster-to-cluster) portals
    //   format: <nVerts> <clusterA> <clusterB> <hint>  (x y z) ...
    // ─────────────────────────────────────────────────────────────────────────
    for (int p = 0; p < numInteriorPortals; ++p)
    {
        // Skip blank lines (shouldn't be any, but be defensive)
        do {
            if (!std::getline(ss, line)) goto done;
            StripCR(line);
        } while (line.empty());

        {
            std::istringstream ls(line);
            int nVerts = 0, clusterA = 0, clusterB = 0, hint = 0;
            if (!(ls >> nVerts >> clusterA >> clusterB >> hint) || nVerts < 3)
                continue;

            std::vector<glm::vec3> verts;
            if (!ParseVertices(ls, nVerts, verts))
                continue;

            BSPPortal portal;
            portal.planeIndex = -1;          // not encoded in .prt
            portal.frontChild = clusterA;    // reuse fields to store cluster IDs
            portal.backChild = clusterB;
            portal.frontLeafIndex = resolveLeaf(clusterA);
            portal.backLeafIndex = resolveLeaf(clusterB);
            portal.vertices = std::move(verts);
            portal.center = ComputeCenter(portal.vertices);
            portal.bounds = ComputeBounds(portal.vertices);

            portals.push_back(std::move(portal));
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Section 2 — Leaf / solid-boundary portals  (one cluster, no opposite leaf)
    //   format: <nVerts> <cluster>  (x y z) ...
    //
    // backLeafIndex is left -1.  IsLeafPortal() returns false for these, so
    // connectivity queries naturally skip them.
    // ─────────────────────────────────────────────────────────────────────────
    for (int p = 0; p < numLeafPortals; ++p)
    {
        do {
            if (!std::getline(ss, line)) goto done;
            StripCR(line);
        } while (line.empty());

        {
            std::istringstream ls(line);
            int nVerts = 0, cluster = 0;
            if (!(ls >> nVerts >> cluster) || nVerts < 3)
                continue;

            std::vector<glm::vec3> verts;
            if (!ParseVertices(ls, nVerts, verts))
                continue;

            BSPPortal portal;
            portal.planeIndex = -1;
            portal.frontChild = cluster;
            portal.backChild = -1;
            portal.frontLeafIndex = resolveLeaf(cluster);
            portal.backLeafIndex = -1;
            portal.vertices = std::move(verts);
            portal.center = ComputeCenter(portal.vertices);
            portal.bounds = ComputeBounds(portal.vertices);

            portals.push_back(std::move(portal));
        }
    }

done:

    BuildClusterToPortalsMap();

    // Nothing to clean up; portals vector is already in its final state.
    return;
}

void CQuake3BSP::BuildClusterToPortalsMap()
{
    m_clusterToPortals.clear();
    m_portalAdjacency.clear();

    // Compute number of clusters (they are 0..N-1 in real Q3 maps, but we compute max to be safe)
    int maxC = -1;
    for (const auto& leaf : leafs)
        if (leaf.cluster > maxC)
            maxC = leaf.cluster;
    m_numClusters = maxC + 1;

    m_clusterToPortals.resize(m_numClusters);
    m_portalAdjacency.resize(portals.size());

    // Build cluster → portals (exactly once, at map load)
    for (size_t i = 0; i < portals.size(); ++i)
    {
        const BSPPortal& p = portals[i];
        if (!p.IsLeafPortal()) continue;

        const int cf = p.FrontCluster(leafs);
        const int cb = p.BackCluster(leafs);

        if (cf >= 0 && cf < m_numClusters)
            m_clusterToPortals[cf].push_back(static_cast<int>(i));
        if (cb >= 0 && cb < m_numClusters)
            m_clusterToPortals[cb].push_back(static_cast<int>(i));
    }

    // ── Pre-build FULL adjacency list (the real win for your "tens of sectors per room" case) ──
    // For every cluster we connect every pair of its portals with bidirectional edges.
    // This is O(Σ K²) but done ONLY ONCE at load time. No more hot-loop work.
    for (int c = 0; c < m_numClusters; ++c)
    {
        const auto& list = m_clusterToPortals[c];
        const size_t k = list.size();
        for (size_t a = 0; a < k; ++a)
        {
            const int u = list[a];
            for (size_t b = a + 1; b < k; ++b)   // avoid self-loops + duplicates
            {
                const int v = list[b];
                m_portalAdjacency[u].push_back(v);
                m_portalAdjacency[v].push_back(u);
            }
        }
    }

    // Optional: sort and unique the adjacency lists if you want (not needed, we never add dups)
}

// ─────────────────────────────────────────────────────────────────────────────
// FindPath
//
// Returns the shortest 3D path from `start` to `target` through the BSP portal
// graph using two passes:
//
//   Pass 1 – Dijkstra on the pre-baked portal adjacency graph finds the
//            optimal sequence of portals to traverse (centre-to-centre cost).
//
//   Pass 2 – Simple Stupid Funnel Algorithm (SSFA) narrows the portal-centre
//            chain into the geometrically shortest path through each portal's
//            actual convex polygon.  Each gate is computed lazily relative to
//            the current funnel apex, so the algorithm stays O(n) in the
//            number of portals even after apex resets.
//
//   Optional – Centripetal Catmull-Rom smoothing rounds the sharp corners
//            produced by the funnel into a continuous curve.
//
// Parameters
//   start        World-space starting position.
//   target       World-space destination.
//   smoothSubdiv Number of intermediate points inserted per waypoint pair by
//                the Catmull-Rom pass.  0 (default) disables smoothing.
//   smoothAlpha  Catmull-Rom tension.  0.5 (default) = centripetal (no cusps).
//                Range [0, 1]; 0 = uniform, 1 = chordal.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<vec3> CQuake3BSP::FindPath(vec3  start,
    vec3  target,
    int   smoothSubdiv,
    float smoothAlpha)
{
    // ── Trivial cases ──────────────────────────────────────────────────────────
    if (glm::distance(start, target) < 0.001f)
        return { start };

    const int cStart = FindClusterAtPosition(start);
    const int cTarget = FindClusterAtPosition(target);

    if (cStart < 0 || cTarget < 0 || cStart == cTarget)
        return { start, target };

    const size_t nPortals = portals.size();
    if (nPortals == 0 || m_numClusters == 0)
        return { start, target };

    // ── Pass 1: Dijkstra on the portal graph ──────────────────────────────────
    constexpr float INF = std::numeric_limits<float>::infinity();

    std::vector<float> dist(nPortals, INF);
    std::vector<int>   prev(nPortals, -1);

    using PQItem = std::pair<float, int>;
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    // Seed: every portal touching the start cluster
    if (cStart < m_numClusters)
    {
        for (int i : m_clusterToPortals[cStart])
        {
            const float d = glm::distance(start, portals[i].center);
            if (d < dist[i])
            {
                dist[i] = d;
                prev[i] = -2;   // sentinel: came directly from start
                pq.push({ d, i });
            }
        }
    }

    while (!pq.empty())
    {
        const auto [cost, u] = pq.top();
        pq.pop();
        if (cost > dist[u]) continue;

        for (int v : m_portalAdjacency[u])
        {
            const float newCost = cost + glm::distance(portals[u].center, portals[v].center);
            if (newCost < dist[v])
            {
                dist[v] = newCost;
                prev[v] = u;
                pq.push({ newCost, v });
            }
        }
    }

    // Find best portal to exit into the target cluster
    float bestTotalCost = INF;
    int   bestLastPortal = -1;

    if (cTarget < m_numClusters)
    {
        for (int i : m_clusterToPortals[cTarget])
        {
            if (dist[i] == INF) continue;
            const float total = dist[i] + glm::distance(portals[i].center, target);
            if (total < bestTotalCost)
            {
                bestTotalCost = total;
                bestLastPortal = i;
            }
        }
    }

    if (bestLastPortal == -1)
        return { start, target };   // no route found

    // Reconstruct ordered portal sequence (start-side → target-side)
    std::vector<int> portalSequence;
    for (int cur = bestLastPortal; cur != -1; cur = prev[cur])
    {
        portalSequence.push_back(cur);
        if (prev[cur] == -2) break;
    }
    std::reverse(portalSequence.begin(), portalSequence.end());

    // ── Pass 2: String Pulling (Funnel Algorithm) ─────────────────────────────
    //
    // Each portal in the sequence contributes a gate: the (left, right) pair of
    // vertices that are the extreme points of the portal polygon perpendicular to
    // the apex→centroid direction in the XZ plane.
    //
    // Funnel invariant: at any moment the funnel is an hourglass from apex, where
    //   • (apex→leftVtx)  is the LEFT  boundary ray
    //   • (apex→rightVtx) is the RIGHT boundary ray
    //
    // For each new gate (gL, gR):
    //   RIGHT side: TriArea2D(apex, rightVtx, gR) >= 0  → gR tightens right, OR
    //               gR crossed left boundary → emit leftVtx corner, restart.
    //   LEFT  side: TriArea2D(apex, leftVtx, gL) <= 0   → gL tightens left, OR
    //               gL crossed right boundary → emit rightVtx corner, restart.
    //
    // The target is appended as a degenerate gate (target, target) to naturally
    // close the funnel; a duplicate-guard prevents double-adding it.
    //
    // Gates are recomputed lazily from the current apex, so after each restart
    // the orientation is always correct relative to the new apex position.
    // ─────────────────────────────────────────────────────────────────────────

    const int numGates = static_cast<int>(portalSequence.size());

    std::vector<glm::vec3> path;
    path.reserve(numGates + 2);
    path.push_back(start);

    // Helper: append p only if it differs from the last waypoint.
    // Uses squared-distance to avoid a sqrt on a hot path.
    const auto addWaypoint = [&](const glm::vec3& p)
        {
            const glm::vec3 d = path.back() - p;
            if (glm::dot(d, d) > 1e-8f)
                path.push_back(p);
        };

    // Funnel state
    glm::vec3 apex = start;
    glm::vec3 leftVtx = start;
    glm::vec3 rightVtx = start;
    int       leftIdx = 0;   // portalSequence index that produced the current leftVtx
    int       rightIdx = 0;   // portalSequence index that produced the current rightVtx

    // Gate accessor — uses the live apex so restarts stay correct.
    // gi == numGates is the synthetic "target" closing gate.
    const auto getGate = [&](int gi) -> std::pair<glm::vec3, glm::vec3>
        {
            if (gi >= numGates) return { target, target };
            return FindPortalGate(portals[portalSequence[gi]].vertices, apex);
        };

    int i = 0;
    while (i <= numGates)
    {
        auto [gL, gR] = getGate(i);

        // ── RIGHT boundary ────────────────────────────────────────────────────
        // TriArea2D(apex, rightVtx, gR) >= 0 means gR is to the LEFT of
        // (apex→rightVtx), i.e. it tightens (or equals) the right boundary.
        if (TriArea2D(apex, rightVtx, gR) >= 0.f)
        {
            if (apex == rightVtx || TriArea2D(apex, leftVtx, gR) < 0.f)
            {
                // gR is within the funnel: accept as new right boundary.
                rightVtx = gR;
                rightIdx = i;
            }
            else
            {
                // gR has crossed the LEFT boundary.
                // The string bends around leftVtx → emit it as a corner,
                // then restart the funnel from just past the portal that
                // provided the left boundary.
                addWaypoint(leftVtx);
                apex = leftVtx;
                leftVtx = apex;
                rightVtx = apex;
                i = leftIdx + 1;
                leftIdx = i;
                rightIdx = i;
                continue;   // skip ++i; reprocess from the new position
            }
        }

        // ── LEFT boundary ─────────────────────────────────────────────────────
        // TriArea2D(apex, leftVtx, gL) <= 0 means gL is to the RIGHT of
        // (apex→leftVtx), i.e. it tightens (or equals) the left boundary.
        if (TriArea2D(apex, leftVtx, gL) <= 0.f)
        {
            if (apex == leftVtx || TriArea2D(apex, rightVtx, gL) > 0.f)
            {
                // gL is within the funnel: accept as new left boundary.
                leftVtx = gL;
                leftIdx = i;
            }
            else
            {
                // gL has crossed the RIGHT boundary.
                // The string bends around rightVtx → emit it as a corner,
                // then restart the funnel from just past the portal that
                // provided the right boundary.
                addWaypoint(rightVtx);
                apex = rightVtx;
                leftVtx = apex;
                rightVtx = apex;
                i = rightIdx + 1;
                leftIdx = i;
                rightIdx = i;
                continue;   // skip ++i
            }
        }

        ++i;
    }

    // Close the path. addWaypoint deduplicates if the funnel already emitted target.
    addWaypoint(target);

    // ── Optional: Catmull-Rom smoothing ───────────────────────────────────────
    if (smoothSubdiv > 0 && path.size() >= 2)
        path = SmoothCatmullRom(path, smoothSubdiv, smoothAlpha);

    return path;
}