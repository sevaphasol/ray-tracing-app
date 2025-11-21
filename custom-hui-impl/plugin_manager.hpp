#pragma once

#include <cstddef>
#include <dlfcn.h>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "cum/ifc/pp.hpp"
#include "cum/plugin.hpp"
#include "dr4/window.hpp"

#include "cum/ifc/dr4.hpp"

namespace cum {

class PluginManager {
  public:
    void
    setupDR4( Plugin* plg )
    {
        backend_dr4_.reset( dynamic_cast<cum::DR4BackendPlugin*>( plg ) );

        if ( backend_dr4_ == nullptr )
        {
            throw std::runtime_error( "Failed dynamic cast dr4backend" );
        }

        window_ = backend_dr4_->CreateWindow();
    }

    void
    setupPP( Plugin* plg )
    {
        backend_pp_.reset( dynamic_cast<cum::PPToolPlugin*>( plg ) );

        if ( backend_pp_ == nullptr )
        {
            throw std::runtime_error( "Failed dynamic cast ppbackend" );
        }
    }

    cum::PPToolPlugin*
    getPluginPP()
    {
        return backend_pp_.get();
    }

    const dr4::Window*
    getWindow() const
    {
        return window_;
    }

    dr4::Window*
    getWindow()
    {
        return window_;
    }

  private:
    std::unique_ptr<cum::DR4BackendPlugin> backend_dr4_;
    std::unique_ptr<cum::PPToolPlugin>     backend_pp_;

    dr4::Window* window_;
};

} // namespace cum
