#pragma once

#include <string>
#include <vector>
#include <map>
#include <cmath>

namespace vfep {

/**
 * @brief Single grid point in CFD field
 */
struct GridPoint {
    double x, y, z;        ///< Position (m)
    double T_K;            ///< Temperature (K)
    double u, v, w;        ///< Velocity components (m/s)
    double rho_kg_m3;      ///< Density (kg/m³)
    double P_Pa;           ///< Pressure (Pa)
};

/**
 * @brief Comparison statistics between VFEP and CFD results
 */
struct ComparisonStats {
    double mean_error;      ///< Mean absolute error
    double max_error;       ///< Maximum absolute error
    double rmse;            ///< Root mean square error
    double correlation;     ///< Correlation coefficient
    int num_points;         ///< Number of comparison points
};

/**
 * @brief CFD coupling interface for VFEP
 * 
 * Provides functionality to:
 * - Import velocity/temperature fields from CFD solvers (VTK format)
 * - Export VFEP results for comparison
 * - Interpolate field values at arbitrary points
 * - Generate comparison statistics and reports
 * 
 * Note: This is a mock interface for Phase 8. No actual CFD solver integration.
 */
class CFDInterface {
public:
    CFDInterface();
    ~CFDInterface() = default;
    
    /**
     * @brief Import velocity field from VTK file
     * @param vtk_file Path to VTK file
     * @return true if successful
     */
    bool importVelocityField(const std::string& vtk_file);
    
    /**
     * @brief Import temperature field from VTK file
     * @param vtk_file Path to VTK file
     * @return true if successful
     */
    bool importTemperatureField(const std::string& vtk_file);
    
    /**
     * @brief Export VFEP results to VTK format
     * @param output_vtk Output file path
     * @param grid_points Vector of grid points to export
     * @return true if successful
     */
    bool exportResults(const std::string& output_vtk, 
                      const std::vector<GridPoint>& grid_points);
    
    /**
     * @brief Export comparison results to CSV
     * @param csv_file Output CSV file path
     * @param vfep_points VFEP results
     * @param cfd_points CFD results (must correspond 1:1)
     * @return true if successful
     */
    bool exportComparisonCSV(const std::string& csv_file,
                            const std::vector<GridPoint>& vfep_points,
                            const std::vector<GridPoint>& cfd_points);
    
    /**
     * @brief Interpolate temperature at point using trilinear interpolation
     * @param x X coordinate (m)
     * @param y Y coordinate (m)
     * @param z Z coordinate (m)
     * @return Interpolated temperature (K)
     */
    double interpolateTemperature(double x, double y, double z) const;
    
    /**
     * @brief Interpolate velocity at point using trilinear interpolation
     * @param x X coordinate (m)
     * @param y Y coordinate (m)
     * @param z Z coordinate (m)
     * @param u Output: u-velocity (m/s)
     * @param v Output: v-velocity (m/s)
     * @param w Output: w-velocity (m/s)
     */
    void interpolateVelocity(double x, double y, double z,
                            double& u, double& v, double& w) const;
    
    /**
     * @brief Compare temperature fields between VFEP and CFD
     * @param vfep_points VFEP results
     * @param cfd_points CFD results (must correspond 1:1)
     * @return Comparison statistics
     */
    ComparisonStats compareTemperature(const std::vector<GridPoint>& vfep_points,
                                      const std::vector<GridPoint>& cfd_points) const;
    
    /**
     * @brief Compare velocity fields between VFEP and CFD
     * @param vfep_points VFEP results
     * @param cfd_points CFD results (must correspond 1:1)
     * @return Comparison statistics
     */
    ComparisonStats compareVelocity(const std::vector<GridPoint>& vfep_points,
                                   const std::vector<GridPoint>& cfd_points) const;
    
    /**
     * @brief Generate mock CFD data for testing
     * @param output_vtk Output file path
     * @param nx Number of points in x
     * @param ny Number of points in y
     * @param nz Number of points in z
     * @param scenario Scenario name ("room_fire", "tunnel", etc.)
     * @return true if successful
     */
    static bool generateMockCFD(const std::string& output_vtk,
                               int nx, int ny, int nz,
                               const std::string& scenario);
    
    /**
     * @brief Get number of grid points loaded
     * @return Number of grid points
     */
    size_t gridPointCount() const { return grid_.size(); }
    
    /**
     * @brief Clear all loaded data
     */
    void clear();
    
private:
    std::vector<GridPoint> grid_;  ///< Loaded grid points
    
    // Grid dimensions for structured grids
    int nx_, ny_, nz_;
    double dx_, dy_, dz_;
    double x_min_, y_min_, z_min_;
    
    /**
     * @brief Parse VTK file (simplified parser)
     * @param vtk_file Path to VTK file
     * @return true if successful
     */
    bool parseVTK(const std::string& vtk_file);
    
    /**
     * @brief Write VTK file (simplified writer)
     * @param vtk_file Output path
     * @param points Grid points to write
     * @return true if successful
     */
    bool writeVTK(const std::string& vtk_file, 
                  const std::vector<GridPoint>& points) const;
    
    /**
     * @brief Find grid cell containing point
     * @param x, y, z Point coordinates
     * @param i0, j0, k0 Output: cell indices
     * @return true if point is inside grid
     */
    bool findCell(double x, double y, double z,
                 int& i0, int& j0, int& k0) const;
    
    /**
     * @brief Trilinear interpolation helper
     * @param c000-c111 Values at 8 corners of cell
     * @param fx, fy, fz Fractional positions in cell [0,1]
     * @return Interpolated value
     */
    double trilinear(double c000, double c100, double c010, double c110,
                    double c001, double c101, double c011, double c111,
                    double fx, double fy, double fz) const;
};

} // namespace vfep
