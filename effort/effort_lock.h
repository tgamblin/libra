#ifndef EFFORT_LOCK_H
#define EFFORT_LOCK_H

namespace effort {
  ///
  /// This class is used to guard against bad MPI implementations that
  /// make MPI calls internally.  This can cause some of the internal
  /// communication done by the effort library to trigger the
  /// wrappers, screwing up the state.
  ///
  class effort_lock {
  public:
    /// On construct, mark that we are effort code.
    effort_lock() {
      lock = true;
    }
    
    /// On destory, mark that we've left the effort library.
    ~effort_lock() {
      lock = false;
    }
    
    /// Use to test whether we're in an effort region or not.
    static bool in_effort() {
      return lock;
    }
    
  private:
    ///
    static bool lock;
  };
  
} // namespace

#endif //EFFORT_LOCK_H
