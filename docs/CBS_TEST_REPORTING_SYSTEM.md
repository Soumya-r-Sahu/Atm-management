# CBS Project - Test Results Reporting System

## Version 1.0 - May 2025

## Overview

The Test Results Reporting System provides a comprehensive framework for capturing, storing, analyzing and visualizing test results from all automated tests within the Core Banking System (CBS) project. This documentation explains the system's components, how to use them, and how they work together to provide insights into test execution.

## Key Features

1. **Multi-Format Test Result Storage**
   - Text log files for human readability
   - JSON for machine processing and integration
   - HTML reports with interactive visualizations

2. **Test Result Analysis**
   - Trend analysis across multiple test runs
   - Statistical analysis of execution times
   - Performance regression detection

3. **Visualization**
   - Interactive HTML reports
   - Test result distribution charts
   - Execution time trends

4. **Alerting**
   - Performance regression detection
   - Alert file generation for significant regressions
   - Configurable regression thresholds

## System Components

### 1. Result Storage Mechanism

Test results are stored in the `tests/Result` directory in three formats:

- **Log Files (.log)**: Human-readable text logs with test details
- **JSON Files (.json)**: Structured data for programmatic analysis
- **HTML Reports (.html)**: Interactive visual reports

Each test run generates files with the naming pattern: `<TestSuiteName>_<Timestamp>.<extension>`

### 2. Result Generation

The `common_test_utils.c` file contains functions to:

- Initialize the test framework
- Track test execution and results
- Generate result files in multiple formats

Key functions:
- `initTestFramework(const char* testSuiteName)` - Initialize the test framework
- `addTestResult(const char* testName, int status, const char* message)` - Add a test result
- `finalizeTestResults()` - Generate result files

### 3. HTML Report Generation

The `html_report_template.h` file provides functions for generating consistent HTML reports:

- `write_html_header()` - Generate HTML document header with styling
- `write_html_summary()` - Generate test summary section with metrics
- `write_html_footer()` - Complete the HTML document

Features:
- Interactive test detail toggles
- Chart.js visualizations
- Responsive design

### 4. Test Result Analysis

The `test_report_analyzer.c` tool analyzes test results to:

- Track pass/fail trends over time
- Generate trend reports with visualizations
- Identify performance patterns

Usage:
```
bin/testing/test_analyzer.exe [--dir <directory>] [--threshold <percent>]
```

### 5. Performance Regression Detection

The `detect_performance_regressions.h` and `performance_regression_check.c` components provide:

- Detection of significant execution time regressions
- Configurable regression thresholds
- Detailed regression reporting

Usage:
```
bin/testing/check_performance.exe [--dir <directory>] [--threshold <percent>] [--verbose]
```

### 6. Automated Analysis Script

The `run_test_analysis.ps1` PowerShell script:

- Runs all test suites
- Analyzes results using the tools above
- Generates alerts for detected regressions

Usage:
```powershell
.\tests\run_test_analysis.ps1
```

## How to Use the System

### Running Tests with Result Generation

All tests that use the common test utilities automatically generate results:

```c
// Example test
#include "tests/common_test_utils.h"

int main(int argc, char* argv[]) {
    initTestFramework("MathTests");
    
    // Run tests
    int result1 = testAddition(5, 3, 8);
    addTestResult("Addition Test", result1 == 0 ? TEST_PASSED : TEST_FAILED, 
                 result1 == 0 ? NULL : "Expected 8, got something else");
                 
    // More tests...
    
    finalizeTestResults();
    return 0;
}
```

### Analyzing Test Results

After running tests, you can analyze the results:

1. **View Individual HTML Reports**
   - Open any `.html` file from the `tests/Result` directory

2. **Generate a Trend Report**
   ```
   bin/testing/test_analyzer.exe
   ```

3. **Check for Performance Regressions**
   ```
   bin/testing/check_performance.exe
   ```

4. **Run Complete Analysis**
   ```powershell
   .\tests\run_test_analysis.ps1
   ```

### Integration with VS Code

VS Code tasks are available for test execution and analysis:

- "Run Performance Regression Check" - Runs the performance checker
- "Test and Analyze Results" - Runs the automated analysis script

## JSON Output Format

The JSON output follows this structure:

```json
{
  "testSuite": "SuiteName",
  "timestamp": "YYYY-MM-DD_HHMMSS",
  "summary": {
    "total": 10,
    "passed": 8,
    "failed": 1,
    "skipped": 1
  },
  "testCases": [
    {
      "name": "Test Case 1",
      "status": "PASSED",
      "executionTime": 0.123,
      "message": null
    },
    {
      "name": "Test Case 2",
      "status": "FAILED",
      "executionTime": 0.456,
      "message": "Expected value was not returned"
    }
  ]
}
```

## HTML Report Structure

The HTML reports include:

1. **Header Section**
   - Test suite name and timestamp
   - Generation date

2. **Summary Section**
   - Metric cards for total, passed, failed, and skipped tests
   - Pass rate calculation
   - Execution time summary

3. **Visualization Section**
   - Test result distribution chart
   - Execution time trend (for trend reports)

4. **Test Details Section**
   - Collapsible details for each test
   - Status indicators
   - Failure messages

## Future Enhancements

1. **Integration with CI/CD pipelines**
   - Jenkins integration for test result publishing
   - GitHub Actions integration

2. **Email notifications**
   - Configurable email alerts for regressions
   - Daily test summary reports

3. **Test coverage integration**
   - Combine with code coverage data
   - Track coverage trends over time

4. **Additional output formats**
   - CSV output for spreadsheet analysis
   - XML output for wider tool integration

5. **Extended historical analysis**
   - Long-term trend analysis
   - Statistical significance testing

## Troubleshooting

### PowerShell Execution Policy

If the PowerShell script fails to run due to execution policy restrictions, use:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_test_analysis.ps1
```

### Missing Results Directory

If results aren't being stored, check that the `tests/Result` directory exists:

```powershell
mkdir -Force tests\Result
```

### Chart Rendering Issues

If charts aren't rendering in HTML reports, check that the internet connection is available for loading Chart.js from CDN.

## Conclusion

The Test Results Reporting System provides comprehensive tools for capturing, analyzing, and visualizing test results in the CBS project. By leveraging multiple output formats and analysis capabilities, it helps identify issues early and ensures the quality of the banking system over time.
