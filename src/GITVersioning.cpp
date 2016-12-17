#include "general.h"
#include "Versioning.h"
#include "Shared.h"
#include "Config.h"

GITVersioning::GITVersioning() : VersioningBase(VERSIONING_GIT)
{
    //
}

bool GITVersioning::VerifyToolchainPresence()
{
    // execute "git help", because its return code is 0 if git toolchain is present
    const char* argv[] = { "git", "help", nullptr };

    return (ExecAndWait(argv[0], argv) == 0);
}

uint32_t GITVersioning::GetRevisionNumber()
{
    if (!CheckOrCreateRepository())
        return 0;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));
    std::string gitParam = "--git-dir="+localPath+".git";
    std::string dirParam = "--work-tree="+localPath+"";

    const char* revCountArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "rev-list", "--count", "HEAD", nullptr };

    int retval;
    std::string output = ExecAndGetOutput(revCountArgv[0], revCountArgv, retval);

    if (retval != 0)
        return 0;

    try
    {
        return (uint32_t)parseLong_ex(output.c_str());
    }
    catch (std::invalid_argument &ex)
    {
        return 0;
    }

    return 0;
}

std::string GITVersioning::GetRevisionHash()
{
    if (!CheckOrCreateRepository())
        return 0;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));
    std::string gitParam = "--git-dir="+localPath+".git";
    std::string dirParam = "--work-tree="+localPath+"";

    const char* revCountArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "rev-parse", "HEAD", nullptr };

    int retval;
    std::string output = ExecAndGetOutput(revCountArgv[0], revCountArgv, retval);

    if (retval != 0)
        return "";

    return output;
}

void GITVersioning::Commit()
{
    if (!CheckOrCreateRepository())
        return;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));
    std::string gitParam = "--git-dir="+localPath+".git";
    std::string dirParam = "--work-tree="+localPath+"";

    // add all files for commit
    const char* addFilesArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "add", localPath.c_str(), nullptr };

    if (ExecAndWait(addFilesArgv[0], addFilesArgv) != 0)
    {
        std::cerr << "Could not add files from local repository instance" << std::endl;
        return;
    }

    // commit!
    const char* commitArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "commit", "-a", "-m", "Database change commit", nullptr };

    if (ExecAndWait(commitArgv[0], commitArgv) != 0)
    {
        std::cerr << "Could not commit files in local repository instance" << std::endl;
        return;
    }
}

void GITVersioning::Push()
{
    if (!CheckOrCreateRepository())
        return;

    std::string baseRemotePath = sConfig->GetConfigStringValue(CONF_VERSION_REPO);
    std::string remotePath = str_trim(baseRemotePath);

    // in GIT, it's possible to maintain local-only repository
    if (remotePath.length() == 0)
        return;

    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));
    std::string gitParam = "--git-dir="+localPath+".git";
    std::string dirParam = "--work-tree="+localPath+"";

    // add all files for commit
    const char* upArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "push", "origin", "master", nullptr };

    if (ExecAndWait(upArgv[0], upArgv) != 0)
    {
        std::cerr << "Could not push changes from local repository instance" << std::endl;
        return;
    }
}

bool GITVersioning::CheckOrCreateRepository()
{
    std::string localPath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));
    std::string baseRemotePath = sConfig->GetConfigStringValue(CONF_VERSION_REPO);
    std::string remotePath = str_trim(baseRemotePath);
    std::string gitParam = "--git-dir="+localPath+".git";
    std::string dirParam = "--work-tree="+localPath+"";

    // probe using svn log - returns 0 on success, 1 if not repository
    const char* logCheckArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "log", nullptr };

    if (ExecAndWait(logCheckArgv[0], logCheckArgv) == 0)
        return true;

    // remote path entered - clone
    if (remotePath.length() > 0)
    {
        const char* cloneArgv[] = { "git", "clone", remotePath.c_str(), localPath.c_str(), nullptr };

        if (ExecAndWait(cloneArgv[0], cloneArgv) != 0)
        {
            std::cerr << "Could not clone remote GIT repository!" << std::endl;
            return false;
        }

        // execute git log once more - returns something else than 0 on empty clone; if 0, it's safe to return
        if (ExecAndWait(logCheckArgv[0], logCheckArgv) == 0)
            return true;

        // will continue under this block
    }
    else // remote path not entered (local only) - init
    {
        const char* cloneArgv[] = { "git", "init", localPath.c_str(), nullptr };

        if (ExecAndWait(cloneArgv[0], cloneArgv) != 0)
        {
            std::cerr << "Could not initialize local GIT repository!" << std::endl;
            return false;
        }
    }

    const char* commEmptyArgv[] = { "git", gitParam.c_str(), dirParam.c_str(), "commit", "--allow-empty", "-m", "Initial empty commit", nullptr };

    if (ExecAndWait(commEmptyArgv[0], commEmptyArgv) != 0)
    {
        std::cerr << "Could not create empty base commit!" << std::endl;
        return false;
    }

    return true;
}
