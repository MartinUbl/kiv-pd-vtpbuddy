#include "general.h"
#include "Versioning.h"
#include "Shared.h"
#include "Config.h"

SVNVersioning::SVNVersioning() : VersioningBase(VERSIONING_SVN)
{
    //
}

bool SVNVersioning::VerifyToolchainPresence()
{
    // execute "svn help", because its return code is 0 if svn toolchain is present
    const char* argv[] = { "svn", "help", nullptr };
    // also execute "svnadmin help", due to need of two separate tools for repository administration
    const char* argv2[] = { "svnadmin", "help", nullptr };
    // we would also like to have svnversion for versioning info
    const char* argv3[] = { "svnversion", nullptr };

    return (ExecAndWait(argv[0], argv) == 0) && (ExecAndWait(argv2[0], argv2) == 0) && (ExecAndWait(argv3[0], argv3) == 0);
}

uint32_t SVNVersioning::GetRevisionNumber()
{
    if (!CheckOrCreateRepository())
        return 0;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));

    const char* versionArgv[] = { "svnversion", localPath.c_str(), nullptr };

    int retval = -1;
    std::string result = ExecAndGetOutput(versionArgv[0], versionArgv, retval);

    if (retval != 0)
        return 0;

    size_t pos;
    if ((pos = result.find_first_of(':')) != std::string::npos)
        pos = pos+1;
    else
        pos = 0;

    return (uint32_t)std::stoll(result.substr(pos));
}

std::string SVNVersioning::GetRevisionHash()
{
    // SVN does not support revision hash, so just return stringified version

    return std::to_string(GetRevisionNumber());
}

void SVNVersioning::Commit()
{
    if (!CheckOrCreateRepository())
        return;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));

    // add all files for commit
    const char* addFilesArgv[] = { "svn", "add", "--force", localPath.c_str(), nullptr };

    if (ExecAndWait(addFilesArgv[0], addFilesArgv) != 0)
    {
        std::cerr << "Could not add files from local repository instance" << std::endl;
        return;
    }

    // commit!
    const char* commitArgv[] = { "svn", "commit", localPath.c_str(), "-m", "Database change commit", nullptr };

    if (ExecAndWait(commitArgv[0], commitArgv) != 0)
    {
        std::cerr << "Could not commit files in local repository instance" << std::endl;
        return;
    }
}

void SVNVersioning::Push()
{
    if (!CheckOrCreateRepository())
        return;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));

    // add all files for commit
    const char* upArgv[] = { "svn", "up", localPath.c_str(), nullptr };

    if (ExecAndWait(upArgv[0], upArgv) != 0)
    {
        std::cerr << "Could not push changes from local repository instance" << std::endl;
        return;
    }
}

bool SVNVersioning::CheckOrCreateRepository()
{
    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));
    std::string remotePath = sConfig->GetConfigStringValue(CONF_VERSION_REPO);

    if (remotePath[remotePath.length() - 1] != '/')
        remotePath += "/";

    // probe using svn log - returns 0 on success, 1 if not repository
    const char* logCheckArgv[] = { "svn", "log", localPath.c_str(), nullptr };

    if (ExecAndWait(logCheckArgv[0], logCheckArgv) == 0)
        return true;

    // checkout remote repository - again returns 0 on success, 1 if no repository at this path is present
    const char* checkoutArgv[] = { "svn", "checkout", (remotePath + "trunk").c_str(), localPath.c_str(), nullptr };

    if (ExecAndWait(checkoutArgv[0], checkoutArgv) == 0)
        return true;

    // if we are here, it means the remote repository does not exist - we need to check, if it's local path
    // (by verifying file:// prefix) and if yes, attempt to create repository and directory tree here

    if (remotePath.substr(0, 7) != "file://")
    {
        std::cerr << "SVN remote repository unreachable, cannot commit" << std::endl;
        return false;
    }

    std::string subPath = remotePath.substr(7);

    std::string absPath = SanitizePath(subPath.c_str());
    if (absPath == "")
    {
        std::cerr << "Invalid SVN repository path specified" << std::endl;
        return false;
    }

    bool exists = true;

    struct stat info;
    if (stat(absPath.c_str(), &info) != 0)
        exists = false;
    else if (!(info.st_mode & S_IFDIR))
        exists = false;

    if (!exists)
    {
        if (mkdir(absPath.c_str(), 0777) != 0)
        {
            std::cerr << "Could not open or create SVN repository directory!" << std::endl;
            return false;
        }
    }

    // now we can be sure the directory exists

    const char* createArgv[] = { "svnadmin", "create", absPath.c_str(), nullptr };

    if (ExecAndWait(createArgv[0], createArgv) != 0)
    {
        std::cerr << "Could not create SVN repository at selected location!" << std::endl;
        return false;
    }

    const char* createDirsArgv[] = { "svn", "mkdir", "-m", "Create directory structure", ("file://" + absPath + "trunk").c_str(), ("file://" + absPath + "branches").c_str(), ("file://" + absPath + "tags").c_str(), nullptr };

    if (ExecAndWait(createDirsArgv[0], createDirsArgv) != 0)
    {
        std::cerr << "Could not create SVN repository structure at selected location!" << std::endl;
        return false;
    }

    // now we are ready to checkout repository to local path

    if (ExecAndWait(checkoutArgv[0], checkoutArgv) != 0)
    {
        std::cerr << "Unable to checkout freshly created repository! Please, check if both local and remote directories has correct rights" << std::endl;
        return false;
    }

    return true;
}
