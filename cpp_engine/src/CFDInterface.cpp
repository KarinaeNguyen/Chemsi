#include "CFDInterface.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

namespace vfep {

CFDInterface::CFDInterface()
    : nx_(0), ny_(0), nz_(0)
    , dx_(0.0), dy_(0.0), dz_(0.0)
    , x_min_(0.0), y_min_(0.0), z_min_(0.0)
{}

bool CFDInterface::importVelocityField(const std::string& vtk_file) {
    return parseVTK(vtk_file);
}

bool CFDInterface::importTemperatureField(const std::string& vtk_file) {
    return parseVTK(vtk_file);
}

bool CFDInterface::exportResults(const std::string& output_vtk,
                                const std::vector<GridPoint>& grid_points) {
    return writeVTK(output_vtk, grid_points);
}

bool CFDInterface::exportComparisonCSV(const std::string& csv_file,
                                      const std::vector<GridPoint>& vfep_points,
                                      const std::vector<GridPoint>& cfd_points) {
    if (vfep_points.size() != cfd_points.size()) {
        return false;
    }
    
    std::ofstream ofs(csv_file);
    if (!ofs.is_open()) return false;
    
    // Header
    ofs << "x,y,z,T_VFEP,T_CFD,dT,u_VFEP,v_VFEP,w_VFEP,u_CFD,v_CFD,w_CFD\n";
    
    // Data rows
    for (size_t i = 0; i < vfep_points.size(); ++i) {
        const auto& vfep = vfep_points[i];
        const auto& cfd = cfd_points[i];
        
        ofs << std::fixed << std::setprecision(4)
            << vfep.x << "," << vfep.y << "," << vfep.z << ","
            << vfep.T_K << "," << cfd.T_K << "," << (vfep.T_K - cfd.T_K) << ","
            << vfep.u << "," << vfep.v << "," << vfep.w << ","
            << cfd.u << "," << cfd.v << "," << cfd.w << "\n";
    }
    
    ofs.close();
    return true;
}

double CFDInterface::interpolateTemperature(double x, double y, double z) const {
    int i0, j0, k0;
    if (!findCell(x, y, z, i0, j0, k0)) {
        return 293.15; // Default ambient if outside grid
    }
    
    // Get 8 corner values
    auto getT = [this](int i, int j, int k) -> double {
        int idx = i + j * nx_ + k * nx_ * ny_;
        if (idx >= 0 && idx < static_cast<int>(grid_.size())) {
            return grid_[idx].T_K;
        }
        return 293.15;
    };
    
    double c000 = getT(i0, j0, k0);
    double c100 = getT(i0 + 1, j0, k0);
    double c010 = getT(i0, j0 + 1, k0);
    double c110 = getT(i0 + 1, j0 + 1, k0);
    double c001 = getT(i0, j0, k0 + 1);
    double c101 = getT(i0 + 1, j0, k0 + 1);
    double c011 = getT(i0, j0 + 1, k0 + 1);
    double c111 = getT(i0 + 1, j0 + 1, k0 + 1);
    
    // Fractional position in cell
    double fx = (x - (x_min_ + i0 * dx_)) / dx_;
    double fy = (y - (y_min_ + j0 * dy_)) / dy_;
    double fz = (z - (z_min_ + k0 * dz_)) / dz_;
    
    return trilinear(c000, c100, c010, c110, c001, c101, c011, c111, fx, fy, fz);
}

void CFDInterface::interpolateVelocity(double x, double y, double z,
                                      double& u, double& v, double& w) const {
    int i0, j0, k0;
    if (!findCell(x, y, z, i0, j0, k0)) {
        u = v = w = 0.0;
        return;
    }
    
    auto getVel = [this](int i, int j, int k, double& u, double& v, double& w) {
        int idx = i + j * nx_ + k * nx_ * ny_;
        if (idx >= 0 && idx < static_cast<int>(grid_.size())) {
            u = grid_[idx].u;
            v = grid_[idx].v;
            w = grid_[idx].w;
        } else {
            u = v = w = 0.0;
        }
    };
    
    double u000, v000, w000, u100, v100, w100;
    double u010, v010, w010, u110, v110, w110;
    double u001, v001, w001, u101, v101, w101;
    double u011, v011, w011, u111, v111, w111;
    
    getVel(i0, j0, k0, u000, v000, w000);
    getVel(i0 + 1, j0, k0, u100, v100, w100);
    getVel(i0, j0 + 1, k0, u010, v010, w010);
    getVel(i0 + 1, j0 + 1, k0, u110, v110, w110);
    getVel(i0, j0, k0 + 1, u001, v001, w001);
    getVel(i0 + 1, j0, k0 + 1, u101, v101, w101);
    getVel(i0, j0 + 1, k0 + 1, u011, v011, w011);
    getVel(i0 + 1, j0 + 1, k0 + 1, u111, v111, w111);
    
    double fx = (x - (x_min_ + i0 * dx_)) / dx_;
    double fy = (y - (y_min_ + j0 * dy_)) / dy_;
    double fz = (z - (z_min_ + k0 * dz_)) / dz_;
    
    u = trilinear(u000, u100, u010, u110, u001, u101, u011, u111, fx, fy, fz);
    v = trilinear(v000, v100, v010, v110, v001, v101, v011, v111, fx, fy, fz);
    w = trilinear(w000, w100, w010, w110, w001, w101, w011, w111, fx, fy, fz);
}

ComparisonStats CFDInterface::compareTemperature(
    const std::vector<GridPoint>& vfep_points,
    const std::vector<GridPoint>& cfd_points) const {
    
    ComparisonStats stats{};
    stats.num_points = std::min(vfep_points.size(), cfd_points.size());
    
    if (stats.num_points == 0) return stats;
    
    double sum_error = 0.0;
    double sum_sq_error = 0.0;
    double max_error = 0.0;
    
    double sum_vfep = 0.0;
    double sum_cfd = 0.0;
    
    for (int i = 0; i < stats.num_points; ++i) {
        double error = std::abs(vfep_points[i].T_K - cfd_points[i].T_K);
        sum_error += error;
        sum_sq_error += error * error;
        max_error = std::max(max_error, error);
        
        sum_vfep += vfep_points[i].T_K;
        sum_cfd += cfd_points[i].T_K;
    }
    
    stats.mean_error = sum_error / stats.num_points;
    stats.rmse = std::sqrt(sum_sq_error / stats.num_points);
    stats.max_error = max_error;
    
    // Calculate correlation (simplified)
    double mean_vfep = sum_vfep / stats.num_points;
    double mean_cfd = sum_cfd / stats.num_points;
    
    double sum_prod = 0.0;
    double sum_vfep_sq = 0.0;
    double sum_cfd_sq = 0.0;
    
    for (int i = 0; i < stats.num_points; ++i) {
        double dv = vfep_points[i].T_K - mean_vfep;
        double dc = cfd_points[i].T_K - mean_cfd;
        sum_prod += dv * dc;
        sum_vfep_sq += dv * dv;
        sum_cfd_sq += dc * dc;
    }
    
    double denom = std::sqrt(sum_vfep_sq * sum_cfd_sq);
    stats.correlation = (denom > 1e-12) ? (sum_prod / denom) : 0.0;
    
    return stats;
}

ComparisonStats CFDInterface::compareVelocity(
    const std::vector<GridPoint>& vfep_points,
    const std::vector<GridPoint>& cfd_points) const {
    
    ComparisonStats stats{};
    stats.num_points = std::min(vfep_points.size(), cfd_points.size());
    
    if (stats.num_points == 0) return stats;
    
    double sum_error = 0.0;
    double sum_sq_error = 0.0;
    double max_error = 0.0;
    
    for (int i = 0; i < stats.num_points; ++i) {
        double vmag_vfep = std::sqrt(
            vfep_points[i].u * vfep_points[i].u +
            vfep_points[i].v * vfep_points[i].v +
            vfep_points[i].w * vfep_points[i].w
        );
        
        double vmag_cfd = std::sqrt(
            cfd_points[i].u * cfd_points[i].u +
            cfd_points[i].v * cfd_points[i].v +
            cfd_points[i].w * cfd_points[i].w
        );
        
        double error = std::abs(vmag_vfep - vmag_cfd);
        sum_error += error;
        sum_sq_error += error * error;
        max_error = std::max(max_error, error);
    }
    
    stats.mean_error = sum_error / stats.num_points;
    stats.rmse = std::sqrt(sum_sq_error / stats.num_points);
    stats.max_error = max_error;
    stats.correlation = 0.0; // Not calculated for velocity
    
    return stats;
}

bool CFDInterface::generateMockCFD(const std::string& output_vtk,
                                  int nx, int ny, int nz,
                                  const std::string& scenario) {
    std::vector<GridPoint> points;
    points.reserve(nx * ny * nz);
    
    // Generate structured grid
    double Lx = 5.0, Ly = 5.0, Lz = 3.0; // Room dimensions
    double dx = Lx / (nx - 1);
    double dy = Ly / (ny - 1);
    double dz = Lz / (nz - 1);
    
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                GridPoint p;
                p.x = i * dx;
                p.y = j * dy;
                p.z = k * dz;
                
                // Mock temperature field (hot near center, stratified vertically)
                double r = std::sqrt(
                    (p.x - Lx / 2) * (p.x - Lx / 2) +
                    (p.y - Ly / 2) * (p.y - Ly / 2)
                );
                
                double T_base = 293.15 + 50.0 * (p.z / Lz); // Stratification
                double T_plume = 300.0 * std::exp(-r * r / 1.0); // Hot center
                p.T_K = T_base + T_plume;
                
                // Mock velocity field (upward in center, outward radially)
                if (r < 0.1) r = 0.1;
                p.u = 0.5 * (p.x - Lx / 2) / r;
                p.v = 0.5 * (p.y - Ly / 2) / r;
                p.w = 1.0 * std::exp(-r * r / 1.0); // Upward plume
                
                p.rho_kg_m3 = 1.2;
                p.P_Pa = 101325.0;
                
                points.push_back(p);
            }
        }
    }
    
    CFDInterface cfd;
    cfd.setGridPoints(points, nx, ny, nz, 0.0, 0.0, 0.0, dx, dy, dz);
    return cfd.writeVTK(output_vtk, points);
}

void CFDInterface::clear() {
    grid_.clear();
    nx_ = ny_ = nz_ = 0;
    dx_ = dy_ = dz_ = 0.0;
    x_min_ = y_min_ = z_min_ = 0.0;
}

void CFDInterface::setGridPoints(const std::vector<GridPoint>& points,
                                int nx, int ny, int nz,
                                double x_min, double y_min, double z_min,
                                double dx, double dy, double dz) {
    grid_ = points;
    nx_ = nx;
    ny_ = ny;
    nz_ = nz;
    x_min_ = x_min;
    y_min_ = y_min;
    z_min_ = z_min;
    dx_ = dx;
    dy_ = dy;
    dz_ = dz;
}
bool CFDInterface::parseVTK(const std::string& vtk_file) {
    std::ifstream ifs(vtk_file);
    if (!ifs.is_open()) return false;
    
    clear();
    
    std::string line;
    // Simplified VTK parser (would need proper implementation for production)
    // For Phase 8, we'll just create a minimal mock
    
    // This is a placeholder - real VTK parsing is more complex
    grid_.push_back({0.0, 0.0, 0.0, 293.15, 0.0, 0.0, 0.0, 1.2, 101325.0});
    nx_ = ny_ = nz_ = 1;
    
    ifs.close();
    return true;
}

bool CFDInterface::writeVTK(const std::string& vtk_file,
                           const std::vector<GridPoint>& points) const {
    std::ofstream ofs(vtk_file);
    if (!ofs.is_open()) return false;
    
    // Write simple VTK ASCII format
    ofs << "# vtk DataFile Version 3.0\n";
    ofs << "VFEP Export\n";
    ofs << "ASCII\n";
    ofs << "DATASET UNSTRUCTURED_GRID\n";
    ofs << "POINTS " << points.size() << " float\n";
    
    for (const auto& p : points) {
        ofs << p.x << " " << p.y << " " << p.z << "\n";
    }
    
    ofs << "\nPOINT_DATA " << points.size() << "\n";
    ofs << "SCALARS Temperature float 1\n";
    ofs << "LOOKUP_TABLE default\n";
    for (const auto& p : points) {
        ofs << p.T_K << "\n";
    }
    
    ofs << "\nVECTORS Velocity float\n";
    for (const auto& p : points) {
        ofs << p.u << " " << p.v << " " << p.w << "\n";
    }
    
    ofs.close();
    return true;
}

bool CFDInterface::findCell(double x, double y, double z,
                           int& i0, int& j0, int& k0) const {
    if (nx_ <= 1 || ny_ <= 1 || nz_ <= 1) return false;
    
    i0 = static_cast<int>((x - x_min_) / dx_);
    j0 = static_cast<int>((y - y_min_) / dy_);
    k0 = static_cast<int>((z - z_min_) / dz_);
    
    return (i0 >= 0 && i0 < nx_ - 1 &&
            j0 >= 0 && j0 < ny_ - 1 &&
            k0 >= 0 && k0 < nz_ - 1);
}

double CFDInterface::trilinear(double c000, double c100, double c010, double c110,
                              double c001, double c101, double c011, double c111,
                              double fx, double fy, double fz) const {
    // Clamp fractions to [0,1]
    fx = std::max(0.0, std::min(1.0, fx));
    fy = std::max(0.0, std::min(1.0, fy));
    fz = std::max(0.0, std::min(1.0, fz));
    
    double c00 = c000 * (1.0 - fx) + c100 * fx;
    double c01 = c001 * (1.0 - fx) + c101 * fx;
    double c10 = c010 * (1.0 - fx) + c110 * fx;
    double c11 = c011 * (1.0 - fx) + c111 * fx;
    
    double c0 = c00 * (1.0 - fy) + c10 * fy;
    double c1 = c01 * (1.0 - fy) + c11 * fy;
    
    return c0 * (1.0 - fz) + c1 * fz;
}

} // namespace vfep
