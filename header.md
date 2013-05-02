# libuv

## main header api

### event loop
#### loop struct
`uv_loop_t` is the struct type of a uv event loop. All async tasks
will be stored in it and then backed up by the event layer system of the os.

##### uv_loop_new
creates a new loop instance.

##### uv_loop_delete
deletes a loop instance.

##### uv_default_loop
returns the current loop.

#### loop management
an instnace of `uv_loop_t` itself does not do anything. First in combination with
the loop mangament functions you can add handles (tasks) to it.

##### uv_run
will start a loop in **mode**.

##### uv_stop
will stop a loop.

##### uv_ref (?)
##### uv_unref (?)
##### uv_has_ref (?)

##### uv_backend_fd
The unix event implementations make the event loop public through a file descriptor.
This method will return it to export it in other libraries.

##### uv_backend_timeout
Time when the os event loop fd will timeout and close.

### misc
#### uv_version
#### uv_version_string
