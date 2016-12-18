#ifndef VTPBUDDY_VERSIONING_H
#define VTPBUDDY_VERSIONING_H

#include "Enums.h"

/**
 * Base class for all versioning implementations
 */
class VersioningBase
{
    public:
        VersioningBase(VersioningTool type) : m_type(type) {};

        // retrieves versioning tool type
        VersioningTool GetType() { return m_type; };

        // verifies the toolchain presence in host system
        virtual bool VerifyToolchainPresence() = 0;

        // retrieves revision number (or commit count)
        virtual uint32_t GetRevisionNumber() = 0;
        // retrieves revision hash
        virtual std::string GetRevisionHash() = 0;

        // retrieves specific file contents from history (minus <history> commits)
        virtual std::string GetFileFromVersion(const char* filename, size_t history = 0, bool relative = false) = 0;

        // creates commit
        virtual void Commit() = 0;

        // pushes to remote repository (if configured properly)
        virtual void Push() = 0;

    protected:
        //

    private:
        // versioning tool type
        VersioningTool m_type;
};

/**
 * SVN versioning class
 */
class SVNVersioning : public VersioningBase
{
    public:
        SVNVersioning();

        virtual bool VerifyToolchainPresence();
        virtual uint32_t GetRevisionNumber();
        virtual std::string GetRevisionHash();
        virtual void Commit();
        virtual void Push();
        virtual std::string GetFileFromVersion(const char* filename, size_t history = 0, bool relative = false);

    protected:
        // Checks for repository structure, and creates it if does not exist
        bool CheckOrCreateRepository();

    private:
        //
};

/**
 * GIT versioning class
 */
class GITVersioning : public VersioningBase
{
    public:
        GITVersioning();

        virtual bool VerifyToolchainPresence();
        virtual uint32_t GetRevisionNumber();
        virtual std::string GetRevisionHash();
        virtual void Commit();
        virtual void Push();
        virtual std::string GetFileFromVersion(const char* filename, size_t history = 0, bool relative = false);

    protected:
        // Checks for repository structure, and creates it if does not exist
        bool CheckOrCreateRepository();

    private:
        //
};

#endif
