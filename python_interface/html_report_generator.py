"""
HTML report generator with logo for VFEP validation results.
"""

import base64
import os
from typing import List, Dict


def encode_logo_base64(logo_path: str) -> str:
    """Convert logo.ico to base64 for embedding in HTML."""
    try:
        with open(logo_path, 'rb') as f:
            return base64.b64encode(f.read()).decode('utf-8')
    except FileNotFoundError:
        return ""


def generate_html_report(validation_results: List[Dict], 
                         output_path: str,
                         logo_path: str = "../Image/logo.ico"):
    """
    Generate HTML validation report with embedded logo.
    
    Args:
        validation_results: List of validation result dictionaries
        output_path: Path to save HTML file
        logo_path: Path to logo.ico file
    """
    logo_base64 = encode_logo_base64(logo_path)
    
    # Calculate summary statistics
    total_scenarios = len(validation_results)
    total_checks = sum(len(r.get('checks', [])) for r in validation_results)
    total_pass = sum(r.get('num_pass', 0) for r in validation_results)
    total_fail = sum(r.get('num_fail', 0) for r in validation_results)
    total_missing = sum(r.get('num_missing', 0) for r in validation_results)
    
    pass_rate = (total_pass / total_checks * 100) if total_checks > 0 else 0
    
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VFEP Validation Report</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }}
        
        .container {{
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 12px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }}
        
        .header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }}
        
        .header-content {{
            flex: 1;
        }}
        
        .header h1 {{
            font-size: 2.5em;
            margin-bottom: 10px;
            font-weight: 300;
        }}
        
        .header p {{
            font-size: 1.1em;
            opacity: 0.9;
        }}
        
        .logo {{
            width: 80px;
            height: 80px;
            margin-left: 20px;
        }}
        
        .summary {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            padding: 40px;
            background: #f8f9fa;
        }}
        
        .stat-card {{
            background: white;
            padding: 25px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            text-align: center;
            transition: transform 0.2s;
        }}
        
        .stat-card:hover {{
            transform: translateY(-5px);
            box-shadow: 0 4px 20px rgba(0,0,0,0.15);
        }}
        
        .stat-value {{
            font-size: 2.5em;
            font-weight: bold;
            margin: 10px 0;
        }}
        
        .stat-label {{
            color: #6c757d;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
        }}
        
        .pass {{ color: #28a745; }}
        .fail {{ color: #dc3545; }}
        .missing {{ color: #ffc107; }}
        
        .results-section {{
            padding: 40px;
        }}
        
        .scenario-card {{
            background: #f8f9fa;
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
            border-left: 4px solid #667eea;
        }}
        
        .scenario-header {{
            font-size: 1.3em;
            font-weight: bold;
            color: #333;
            margin-bottom: 15px;
        }}
        
        .checks-table {{
            width: 100%;
            border-collapse: collapse;
            background: white;
            border-radius: 8px;
            overflow: hidden;
        }}
        
        .checks-table th {{
            background: #667eea;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: 500;
        }}
        
        .checks-table td {{
            padding: 10px 12px;
            border-bottom: 1px solid #dee2e6;
        }}
        
        .checks-table tr:last-child td {{
            border-bottom: none;
        }}
        
        .checks-table tr:hover {{
            background: #f8f9fa;
        }}
        
        .status-badge {{
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 0.85em;
            font-weight: bold;
            text-transform: uppercase;
        }}
        
        .status-pass {{
            background: #d4edda;
            color: #155724;
        }}
        
        .status-fail {{
            background: #f8d7da;
            color: #721c24;
        }}
        
        .status-missing {{
            background: #fff3cd;
            color: #856404;
        }}
        
        .footer {{
            background: #343a40;
            color: white;
            padding: 20px;
            text-align: center;
        }}
        
        .progress-bar {{
            width: 100%;
            height: 30px;
            background: #e9ecef;
            border-radius: 15px;
            overflow: hidden;
            margin: 20px 0;
        }}
        
        .progress-fill {{
            height: 100%;
            background: linear-gradient(90deg, #28a745 0%, #20c997 100%);
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            transition: width 1s ease;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="header-content">
                <h1>VFEP Validation Report</h1>
                <p>Phase 10 - Field Validation & Production Readiness</p>
                <p style="font-size: 0.9em; margin-top: 5px;">Generated: {__import__('datetime').datetime.now().strftime('%B %d, %Y at %H:%M:%S')}</p>
            </div>
"""
    
    if logo_base64:
        html_content += f'            <img src="data:image/x-icon;base64,{logo_base64}" alt="VFEP Logo" class="logo">\n'
    
    html_content += f"""        </div>
        
        <div class="summary">
            <div class="stat-card">
                <div class="stat-label">Scenarios</div>
                <div class="stat-value">{total_scenarios}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Total Checks</div>
                <div class="stat-value">{total_checks}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label pass">Passed</div>
                <div class="stat-value pass">{total_pass}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label fail">Failed</div>
                <div class="stat-value fail">{total_fail}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label missing">Missing</div>
                <div class="stat-value missing">{total_missing}</div>
            </div>
        </div>
        
        <div style="padding: 0 40px;">
            <h2 style="margin-bottom: 10px;">Overall Pass Rate</h2>
            <div class="progress-bar">
                <div class="progress-fill" style="width: {pass_rate}%">{pass_rate:.1f}%</div>
            </div>
        </div>
        
        <div class="results-section">
            <h2 style="margin-bottom: 30px;">Detailed Results</h2>
"""
    
    # Add scenario results
    for result in validation_results:
        if 'error' in result:
            continue
            
        scenario_id = result['scenario_id']
        checks = result['checks']
        
        html_content += f"""            <div class="scenario-card">
                <div class="scenario-header">{scenario_id}</div>
                <table class="checks-table">
                    <thead>
                        <tr>
                            <th>Target Type</th>
                            <th>Target</th>
                            <th>Predicted</th>
                            <th>Bounds</th>
                            <th>Error</th>
                            <th>Status</th>
                        </tr>
                    </thead>
                    <tbody>
"""
        
        for check in checks:
            if check.get('status') == 'MISSING':
                continue
                
            status_class = f"status-{check['status'].lower()}"
            html_content += f"""                        <tr>
                            <td><strong>{check['target_type']}</strong></td>
                            <td>{check['target_value']:.2f} {check['units']}</td>
                            <td>{check['predicted_value']:.2f} {check['units']}</td>
                            <td>[{check['lower_bound']:.2f}, {check['upper_bound']:.2f}]</td>
                            <td>{check['absolute_error']:+.2f} ({check['relative_error']:+.1f}%)</td>
                            <td><span class="status-badge {status_class}">{check['status']}</span></td>
                        </tr>
"""
        
        html_content += """                    </tbody>
                </table>
            </div>
"""
    
    html_content += """        </div>
        
        <div class="footer">
            <p><strong>VFEP - Versatile Fire Engineering Platform</strong></p>
            <p style="margin-top: 10px; font-size: 0.9em;">Copyright © 2026 VFEP Development Team | Phase 10 Complete</p>
        </div>
    </div>
</body>
</html>
"""
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(html_content)
    
    print(f"HTML report with logo generated: {output_path}")
