#include "particlesdefinitions.h"

#include <Tempest/Log>

#include "graphics/pfx/particlefx.h"
#include "utils/fileext.h"
#include "gothic.h"

using namespace Tempest;

ParticlesDefinitions::ParticlesDefinitions() {
  vm = Gothic::inst().createVm("ParticleFx.dat");
  }

ParticlesDefinitions::~ParticlesDefinitions() {
  vm->clearReferences(Daedalus::IC_Pfx);
  }

const ParticleFx* ParticlesDefinitions::get(std::string_view name, bool relaxed) {
  if(name.empty())
    return nullptr;

  while(FileExt::hasExt(name,"PFX"))
    name = name.substr(0,name.size()-4);

  std::lock_guard<std::recursive_mutex> guard(sync);
  return implGet(name,relaxed);
  }

const ParticleFx* ParticlesDefinitions::get(const ParticleFx* base, const VisualFx::Key* key) {
  if(base==nullptr || key==nullptr)
    return base;
  std::lock_guard<std::recursive_mutex> guard(sync);
  return implGet(*base,*key);
  }

const ParticleFx* ParticlesDefinitions::implGet(std::string_view name, bool relaxed) {
  auto cname = std::string(name);
  auto it    = pfx.find(cname);
  if(it!=pfx.end())
    return it->second.get();
  Daedalus::GEngineClasses::C_ParticleFX decl={};
  if(!implGet(name,decl,relaxed))
    return nullptr;
  std::unique_ptr<ParticleFx> p{new ParticleFx(decl,name)};
  auto elt = pfx.insert(std::make_pair(std::move(cname),std::move(p)));

  return elt.first->second.get();
  }

const ParticleFx* ParticlesDefinitions::implGet(const ParticleFx& base, const VisualFx::Key& key) {
  auto it = pfxKey.find(&key);
  if(it!=pfxKey.end())
    return it->second.get();

  std::unique_ptr<ParticleFx> p{new ParticleFx(base,key)};
  auto elt = pfxKey.insert(std::make_pair(&key,std::move(p)));

  return elt.first->second.get();
  }

bool ParticlesDefinitions::implGet(std::string_view name,
                                   Daedalus::GEngineClasses::C_ParticleFX& ret,
                                   bool relaxed) {
  if(!vm || name.empty())
    return false;

  char buf[256] = {};
  std::snprintf(buf,sizeof(buf),"%.*s",int(name.size()),name.data());
  auto id = vm->getDATFile().getSymbolIndexByName(buf);
  if(id==size_t(-1)) {
    if(!relaxed)
      Log::e("invalid particle system: \"",buf,"\"");
    return false;
    }

  vm->initializeInstance(ret, id, Daedalus::IC_Pfx);
  vm->clearReferences(Daedalus::IC_Pfx);
  return true;
  }
