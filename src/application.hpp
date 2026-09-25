#pragma once

/** @file
 *  @brief Public entry point for the Codesoc Cubed desktop application.
 */

namespace codesoc {

/** Parses command-line options, opens the game window, and runs one session.
 *  @return A process exit code suitable for returning from main().
 */
int runApplication(int argc, char** argv);

} // namespace codesoc
